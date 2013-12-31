<?php

/**
 *
 * Drush Module for Data Migration Package
 *
 * Copyright 2013 Onur Rauf Bingol
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * Implementation of hook_drush_command()
 */
function orb_drush_command() {

  $items = array();
  $items['orb-convert-featured-slider'] = array(
    'description' => 'Convert "Featured Slider" content type to fit D7 side',
    'alias' => array('orb-cfs'),
    'arguments' => array(
      'lang' => 'Language of the node, like "en" or "tr" (without quotes)',
    ),
  );
  $items['orb-load-node'] = array(
    'description' => 'Dumps node array elements to screen',
    'alias' => array('orb-ln'),
    'arguments' => array(
      'nid' => 'Node ID to load',
    ),
  );
  $items['orb-convert-video-content'] = array(
    'description' => 'Convert "Video Content" content type to fit D7 side',
    'alias' => array('orb-cvc'),
    'arguments' => array(
      'lang' => 'Language of the node, like "en" or "tr" (without quotes)',
    ),
  );
  $items['orb-fix-translations'] = array(
    'description' => 'Fix translations after migration process',
    'alias' => array('orb-ft'),
  );
  $items['orb-remove-taxonomy-term'] = array(
    'description' => 'Removes taxonomy terms',
    'alias' => array('orb-rtt'),
  );
  $items['orb-fix-forwards'] = array(
    'description' => 'Convert forward_uri field to forward_to field for automatic redirection',
    'alias' => array('orb-ff'),
  );
  $items['orb-unset-forward-uri'] = array(
    'description' => 'Unset forward_uri field',
    'alias' => array('orb-ufu'),
  );

  return $items;
}

/**
 * Drush Command: drush convert-fs $lang (default $lang = en)
 * Updates "Featured Slider" content type with the body of the local linked page
 */
function drush_orb_convert_featured_slider($lang='en') {

  // We are setting up the language of the nodes in consideration
  if (isset($lang)) {

     $current_lang = $lang;

  } else {

    // Probably this error message won't be printed in any case.
    drush_set_error('error', t('Specify language to continue'));

  }

  // We cheated and used the queries from the Views module :-)
  $fs_nodes = db_query("
      SELECT DISTINCT(node.nid) AS nid,
        node_data_field_link.field_link_url AS node_data_field_link_field_link_url,
        node_data_field_link.field_link_title AS node_data_field_link_field_link_title,
        node_data_field_link.field_link_attributes AS node_data_field_link_field_link_attributes,
        node.language AS node_language,
        node.type AS node_type,
        node.vid AS node_vid,
        node.created AS node_created
      FROM node node
        LEFT JOIN content_type_featured_slider node_data_field_link ON node.vid = node_data_field_link.vid
      WHERE (node.status = 1)
        AND (node.type = '%s')
        AND (node.language = '%s')
      GROUP BY nid
      ORDER BY node_created DESC
    ", array('featured_slider', $current_lang)
  );

  $fs_node_bodies = db_query("
      SELECT DISTINCT(node.nid) AS nid,
        node_revisions.body AS node_revisions_body,
        node_revisions.format AS node_revisions_format,
        node.created AS node_created
      FROM node node
        LEFT JOIN node_revisions node_revisions ON node.vid = node_revisions.vid
      WHERE (node.status = 1)
        AND (node.type = '%s')
      GROUP BY nid
      ORDER BY node_created DESC
    ", array('page')
  );

  // Initialize counters
  $count_updated_fs_bodies = 0;
  $count_updated_fs_forward_uri = 0;
  $count_updated_fs_ct = 0;

  // Start iteration of featured slider nodes
  foreach ($fs_nodes as $fsn) {

    // It is better to open the node at the start
    $node = node_load(array("nid" => $fsn['nid']));

    // Because there will be a node update, last update date might be changed. This is not wanted.
    $changed = $node->changed;

    // Check for internal links
    preg_match('~^http[s]?\://[[w]{3}[\.]]?example\.com/(.*)~', $fsn['node_data_field_link_field_link_url'], $matches);

    if (!empty($matches)) { // start $matches check

      // We need to exclude files
      preg_match('~^sites/default/files/(.*)~', $matches[1], $matches_files);

      if (empty($matches_files)) {

        // url_alias table does not store language prefix
        preg_match('~^tr/(.*)~', $matches[1], $matches_lang_tr);

        if (!empty($matches_lang_tr[1])) { // start $matches_lang_tr check

          $lang_prefix = 'tr';
          $alias_path = $matches_lang_tr[1];

        } else { // other conditions in $matches_lang_tr check

          $lang_prefix = 'en';
          $alias_path = $matches[1];

        } // end $matches_lang_tr check

        // Load URL alias using Drupal's internal functtons
        $real_path = drupal_lookup_path('source', $alias_path, $lang_prefix);

        $nid_preprocess = explode('/', $real_path);
        $nid = $nid_preprocess[1];

        $body = null;

        // Iterate through page content types to find the correct body value
        foreach ($fs_node_bodies as $fsnb) {

          if ($fsnb['nid'] == $nid) { // start $fsnb['nid'] check

            $body = $fsnb['node_revisions_body'];
            break;

          } else { // other conditions in $fsnb['nid'] check

            $body = null;

          } // end $fsnb['nid'] check

        } // foreach end $fs_node_bodies

        // If we found the body, set featured_slider body value to this value
        if ($body != null) { // start $body check

          // Fill the body if the link is local
          $node->body = $body;

          // Just to see the count of featured slider nodes which their body is updated.
          $count_updated_fs_bodies++;

        } // end $body check

      } // end $matches_files check

    }  else { // other conditions in $matches check

      // Sometimes it is possible to forgot writing "http" in the links, so check for it
      preg_match('~^http~', $node->field_link[0]['url'], $matches_links);

      // If "http" is forgotten, add it to the front of the link
      if (!empty($matches_links)) { // start $matches_links check

        $forward_uri = $node->field_link[0]['url'];

      } else { // other conditions in $matches_links check

        $forward_uri = "http://" . $node->field_link[0]['url'];

      } // end $matches_links check

      // Fill "Forward URI" if the link is external
      $node->field_forward_uri[0]['url'] = $forward_uri;
      $node->field_forward_uri[0]['title'] = $node->field_link[0]['title'];

      $count_updated_fs_forward_uri++;

    } // end $matches check

    // Update all featured slider nodes' expire date
    $node->field_expire_date[0]['value'] = date('Y-m-d H:i:s', $node->created);

    // Finally, we can save the node
    node_save($node);

    // We need to get the changed date back to its original value
    $node_update = new stdClass();
    $node_update->changed = $changed;
    drupal_write_record('node', $node_update, 'nid');

    $count_updated_fs_ct++;

  } // foreach end $fs_nodes

  // Print some information
  echo "\n";
  drush_print('Updated Body Fields: ' . $count_updated_fs_bodies);
  drush_print('Updated Forward URI Fields: ' . $count_updated_fs_forward_uri);
  drush_print('Updated Featured Slider CTs: ' . $count_updated_fs_ct);

}

/**
 * Drush Command: drush load-node $nid
 * Loads node by its ID and dumps the node variables
 */
function drush_orb_load_node($nid=0) {

  if($nid == 0) {

    drush_set_error('error', t('Specify Node ID to load'));

  } else {

    $node = node_load(array("nid" => $nid));
    drush_print_r($node);

  }

}

/**
 * Drush Command: drush convert-vc $lang (default $lang = en)
 * Extracts Youtube link code and updates Video Embed field in "Video Content" content type
 */
function drush_orb_convert_video_content($lang='en') {

  // We are setting up the language of the nodes in consideration
  if (isset($lang)) {

    $current_lang = $lang;

  } else {

    // Probably this error message won't be printed in any case.
    drush_set_error('error', t('Specify language to continue'));

  }

  // We cheated and used the queries from the Views module :-)
  $fs_nodes = db_query("
      SELECT DISTINCT(node.nid) AS nid,
        node_data_field_embedcode.field_embedcode_value AS node_data_field_embedcode_field_embedcode_value,
        node_data_field_embedcode.field_embedcode_format AS node_data_field_embedcode_field_embedcode_format,
        node.language AS node_language,
        node.type AS node_type,
        node.vid AS node_vid,
        node.created AS node_created
      FROM node node
      LEFT JOIN content_type_video_content node_data_field_embedcode
        ON node.vid = node_data_field_embedcode.vid
      WHERE (node.status = 1)
        AND (node.type = '%s')
        AND (node.language = '%s')
      GROUP BY nid
      ORDER BY node_created DESC
      ", array('video_content', $current_lang)
  );

  // Counters are only for information
  $count_updated_video_embed_fields = 0;
  $count_not_updated_video_embed_fields = 0;

  foreach ($fs_nodes as $fsn) { // foreach start $fs_nodes

    // It is better to open the node at the start
    $node = node_load(array("nid" => $fsn['nid']));

    // Because there will be a node update, last update date might be changed. This is not wanted.
    $changed = $node->changed;

    // Match Youtube link code. Ex: xsaFXx3jKJY
    preg_match('~http[s]?\://[[w]{3}[\.]]?youtube\.com/embed/(.*)(?:\?)~', $fsn['node_data_field_embedcode_field_embedcode_value'], $matches);


    if (!empty($matches)) { // start $matches check

      // Fill Video Embed field
      $node->field_video_embed[0]['value'] = 'http://www.youtube.com/watch?v=' . $matches[1];
      $count_updated_video_embed_fields++;

    } else { // other conditions in $matches check

      // When Youtube is not found, increase counter and print a message about it. Note that, this counter should always be zero.
      drush_print('Problem with node: ' . $node->nid);
      $count_not_updated_video_embed_fields++;

    } // end $matches check

    // We can save the node right now
    node_save($node);

    // We need to get the changed date back to its original value
    $node_update = new stdClass();
    $node_update->changed = $changed;
    drupal_write_record('node', $node_update, 'nid');

  } // foreach end $fs_nodes


  // Print collected information
  echo "\n";
  drush_print('Count of updated nodes: ' . $count_updated_video_embed_fields);
  drush_print('Count of nodes that are not updated: ' . $count_not_updated_video_embed_fields);

}

/**
 * Drush Command: drush fix-tr
 * Fixes translation migrations after migrate operations
 */
function drush_orb_fix_translations() {

  // We want this function to run only in Drupal 7 and we check using PHP version_compare() function.
  if (version_compare(VERSION, '7', '<')) {

    drush_set_error('Need Drupal 7 to run');
    return false;

  }

  // Set the counter for informational purposes
  $count = 0;

  // Query D6 nodes table (returns stdClass in D7)
  $old_nodes = Database::getConnection('default', 'source')->query("SELECT DISTINCT(node.nid) AS nid FROM node node WHERE (node.status = 1)");

  // Query translation match table (returns stdClass in D7)
  $translations = db_query("SELECT tn.source_nid AS source_nid, tn.source_tnid AS source_tnid, tn.dest_nid AS dest_nid FROM aa_translation_nodes tn");

  // It just works right in this way. Otherwise, it is slow to use any kind of iterator.
  $trs = array();

  foreach ($translations as $t) { // foreach start $translations

    if ($t->source_tnid != 0) { // start $t->source_tnid check

      $snid = $t->source_nid;
      $trs[$snid]['snid'] = $t->source_nid;
      $trs[$snid]['stnid'] = $t->source_tnid;
      $trs[$snid]['dnid'] = $t->dest_nid;

    } // end $t->source_tnid check

  } // foreach end $translations

  // We want to measure time -starting time-
  $start_time = time();

  foreach ($old_nodes as $on) { // foreach start $old_nodes

    $dest_node = $trs[$on->nid]['dnid'];

    $source_translated_id = $trs[$on->nid]['stnid'];

    $final_dest = $trs[$source_translated_id]['dnid'];

    if (!empty($final_dest)) { // start $final_dest check

      // Load the node
      $node = node_load($dest_node);

      // Set translation ID of the open node
      $node->tnid = $final_dest;

      // We want to measure time -ending time-
      $process_time = time();

      // Print node ID just for printing something to the command line
      $print_var = 'Processed node' . ' ' . $final_dest . ' ' . 'at the second' . ' ' . ($process_time - $start_time) . ' ' . 'from the start';
      drush_print($print_var);

      // Increment the counter
      $count++;

      // We can save the node right now
      node_save($node);

    } else { // other conditions in $final_dest check

      //drush_print($dest_node);

    } // end $final_dest check

  } // foreach end $old_nodes

  // Print informational messages
  drush_print('Processed node count: ' . $count);

  // IDE throws a warning about it :-)
  return true;

}

/**
 * Drush Command: drush del-tax
 * Removes defined taxonomy term from "featured_slider" nodes
 */
function drush_orb_remove_taxonomy_term() {

  // Static definition of taxonomy term ID
  $tax_term_tid = 17;

  // Query featured_slider nodes
  $all_nodes = db_query("SELECT nid AS nid FROM node WHERE type = 'featured_slider'");

  foreach ($all_nodes as $an) { // foreach start $all_nodes

    // Load the node
    $node = node_load($an->nid);

    // We are working with "User Groups" field
    $data = $node->field_user_groups;

    foreach ($data as $key => $value) { // foreach start $data

      foreach ($value as $vkey => $vvalue) { // foreach start $value

        if ($vvalue['tid'] == $tax_term_tid) { // start $vvalue['tid'] check

          // Unset given taxonomy term from loaded node
          unset($data[$key][$vkey]);

        } // end $vvalue['tid'] check

      } // foreach end $value

    } // foreach end $data

    // Set new "User Groups" array
    $node->field_user_groups = $data;

    // Save the opened node
    node_save($node);

  } // foreach end $all_nodes

}

/**
 * Drush Command: drush fix-fws
 * Converts `forward_uri` field to `forward_to` field for redirection
 */
function drush_orb_fix_forwards() {

  // We want this function to run only in Drupal 7 and we check using PHP version_compare() function.
  if (version_compare(VERSION, '7', '<')) {

    drush_set_error('Need Drupal 7 to run');
    return false;

  }

  $nodes = db_query("SELECT nid FROM node WHERE type = 'featured_slider'");

  foreach ($nodes as $n) { // foreach start $nodes

    // Open the node
    $node = node_load($n->nid);

    // Process when `forward_uri` field is filled
    if (!empty($node->field_forward_uri)) { // start `forward_uri` field check

      // Set `forward_to` field
      $forward_uri = $node->field_forward_uri['und'][0]['safe_value'];
      $node->field_forward_to = array();
      $node->field_forward_to['und'] = array();
      $node->field_forward_to['und'][0] = array();
      $node->field_forward_to['und'][0]['url'] = $forward_uri;
      $node->field_forward_to['und'][0]['title'] = '';
      $node->field_forward_to['und'][0]['attributes'] = array();

      // Save the opened node
      node_save($node);

    } // end `forward_uri` field check

  } // foreach end $nodes

}

/**
 * Drush Command: drush unset-fu
 * Sets `forward_uri` field to an empty array
 */
function drush_orb_unset_forward_uri() {

  // We want this function to run only in Drupal 7 and we check using PHP version_compare() function.
  if (version_compare(VERSION, '7', '<')) {

    drush_set_error('Need Drupal 7 to run');
    return false;

  }

  $nodes = db_query("SELECT nid FROM node WHERE type = 'featured_slider'");

  foreach ($nodes as $n) { // foreach start $nodes

    // Open the node
    $node = node_load($n->nid);

    // Process when `forward_uri` field is filled
    unset($node->field_forward_uri);

    // Save the opened node
    node_save($node);

  } // foreach end $nodes

}

