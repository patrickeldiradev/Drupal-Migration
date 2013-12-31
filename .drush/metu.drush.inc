<?php

function metu_drush_command() {
  $items = array();
  $items['metu-convert'] = array(
    'description' => "Convert upload module to cck filefield",
  );
  $items['metu-convert-featured-slider'] = array(
    'description' => 'Convert Featured Slider Content Type',
  );

  return $items;
}

function drush_metu_convert_featured_slider() {

  $current_lang = 'en';

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

  $count = 0;

  foreach ($fs_nodes as $fsn) {

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

          $node = node_load(array("nid" => $fsn['nid']));
          $node->body = $body;
          $date_preprocess = explode('+', date('c', $node->created));
          $node->field_expire_date[0]['value'] = $date_preprocess[0];
          node_save($node);

          /*
          $node_test = node_load(array("nid" => $fsn['nid']));
          var_dump($node_test);
          */

        }

        $count++;

      }

    }

  }

  echo "\n";
  echo 'Count: ' . $count . "\n";

}