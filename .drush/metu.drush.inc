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

  return $items;
}

/*
 * Drush Command: drush convert-fs $lang
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
        node.language AS node_language, node.type AS node_type,
        node.vid AS node_vid,
        node.created AS node_created
      FROM node node
        LEFT JOIN term_node term_node ON node.vid = term_node.vid
        LEFT JOIN term_data term_data ON term_node.tid = term_data.tid
        LEFT JOIN content_field_link node_data_field_link ON node.vid = node_data_field_link.vid
      WHERE (node.status = 1)
        AND (node.type = '%s')
        AND (node.language = '%s')
        AND (term_data.vid in ('5'))
        AND (term_node.tid IN (25, 28, 27, 23, 22, 26, 24))
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

  $count_updated_fs_bodies = 0;
  $count_updated_fs_ct = 0;

  foreach ($fs_nodes as $fsn) {

    // It is better to open the node at the start
    $node = node_load(array("nid" => $fsn['nid']));

    preg_match('~^http[s]?\://[[w]{3}[\.]]?metu\.edu\.tr/(.*)~', $fsn['node_data_field_link_field_link_url'], $matches);

    if (!empty($matches)) {

      preg_match('~^sites/default/files/(.*)~', $matches[1], $matches_files);

      if (empty($matches_files)) {

        preg_match('~^tr/(.*)~', $matches[1], $matches_lang_tr);

        if (!empty($matches_lang_tr[1])) {

          $lang_prefix = 'tr';
          $alias_path = $matches_lang_tr[1];

        } else {

          $lang_prefix = 'en';
          $alias_path = $matches[1];

        }

        $real_path = drupal_lookup_path('source', $alias_path, $lang_prefix);

        $nid_preprocess = explode('/', $real_path);
        $nid = $nid_preprocess[1];

        $body = null;

        foreach ($fs_node_bodies as $fsnb) {

          if ($fsnb['nid'] == $nid) {

            $body = $fsnb['node_revisions_body'];
            break;

          }

        }

        if ($body != null) {

          $node->body = $body;

        } else {

          // just nothing right now

        }


        // Just to see the count of featured slider nodes which their body is updated.
        $count_updated_fs_bodies++;

      }

    }

    // Update all featured slider nodes' expire date
    $node->field_expire_date[0]['value'] = date('Y-m-d H:i:s', $node->created);

    // Finally, we can save the node
    node_save($node);

    $count_updated_fs_ct++;

  }

  echo "\n";
  drush_print('Updated Body Fields: ' . $count_updated_fs_bodies);
  drush_print('Updated Featured Slider CTs: ' . $count_updated_fs_ct);

}

/*
 * Drush Command: drush load-node $nid
 * Loads node by its ID
 *
 */
function drush_metu_load_node($nid=0) {

  if($nid == 0) {

    drush_set_error('error', t('Specify Node ID to load'));

  } else {

    $node = node_load(array("nid" => $nid));
    var_dump($node);

  }

}