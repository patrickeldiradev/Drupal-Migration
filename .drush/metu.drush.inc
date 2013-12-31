<?php

function metu_drush_command() {

  $items = array();
  $items['metu-convert-featured-slider'] = array(
    'description' => 'Convert Featured Slider Content Type',
    'alias' => array('metu-cfs'),
    'arguments' => array(
      'lang' => 'Language of the node, like "en" or "tr" (without quotes)',
    ),
  );
  $items['metu-load-node'] = array(
    'description' => 'Convert Featured Slider Content Type',
    'alias' => array('metu-ln'),
    'arguments' => array(
      'nid' => 'Node ID to load',
    ),
  );
  $items['metu-convert-video-content'] = array(
    'description' => 'Convert "Video Content" Content Type',
    'alias' => array('metu-cvc'),
    'arguments' => array(
      'lang' => 'Language of the node, like "en" or "tr" (without quotes)',
    ),
  );

  return $items;
}

/*
 * Drush Command: drush convert-fs $lang (default $lang = en)
 * Updates "Featured Slider" content type with the body of the local linked page
 *
 */
function drush_metu_convert_featured_slider($lang='en') {

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
        AND (node.language = '%s')
      GROUP BY nid
      ORDER BY node_created DESC
    ", array('page', $current_lang)
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
    preg_match('~^http[s]?\://[[w]{3}[\.]]?metu\.edu\.tr/(.*)~', $fsn['node_data_field_link_field_link_url'], $matches);

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

        } // end $body check

        // Just to see the count of featured slider nodes which their body is updated.
        $count_updated_fs_bodies++;

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

/*
 * Drush Command: drush load-node $nid
 * Loads node by its ID and dumps the node variables
 *
 */
function drush_metu_load_node($nid=0) {

  if($nid == 0) {

    drush_set_error('error', t('Specify Node ID to load'));

  } else {

    $node = node_load(array("nid" => $nid));
    drush_print_r($node);

  }

}

/*
 * Drush Command: drush convert-vc $lang (default $lang = en)
 * Extracts Youtube link code and updates Video Embed field in "Video Content" content type
 *
 */
function drush_metu_convert_video_content($lang='en') {

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