<?php

/**
 *
 * Drush Module for Import from an External Database
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
function extern_drush_command() {

  $items = array();
  $items['extern-import'] = array(
    'description' => 'External import from another database',
    'alias' => array('ext-imp'),
  );

  return $items;

}

/**
 * Drush Command: drush ext-imp
 * Import data to Drupal 7 from an external database. No need to use Migrate module in this case.
 */
function drush_extern_import() {

  $items = Database::getConnection('default', 'source')
    ->query("SELECT title_eng, title_trk, answer_eng, answer_trk, keywords_eng, keywords_trk FROM sss_tum ORDER BY id DESC");

  // Start iteration of external database items
  foreach ($items as $fi) {

    $title_tr = strip_tags($fi->title_trk);
    $title_en = strip_tags($fi->title_eng);
    $bodytext_tr = $fi->answer_trk;
    $bodytext_en = $fi->answer_eng;

    $node_tr = new stdClass();
    $node_tr->type = "page";
    node_object_prepare($node_tr);

    $node_tr->title = $title_tr;
    $node_tr->language = 'tr';

    $node_tr->uid = 1;

    $node_tr->body['und'][0]['value'] = $bodytext_tr;
    $node_tr->body['und'][0]['summary'] = text_summary(strip_tags($bodytext_tr));
    $node_tr->body['und'][0]['format'] = 'full_html';

    if ($node_tr = node_submit($node_tr)) {

      node_save($node_tr);
      echo "TR Node with nid " . $node_tr->nid . " saved!\n";

    }

    $node_en = new stdClass();
    $node_en->type = "page";
    node_object_prepare($node_en);

    $node_en->title = $title_en;
    $node_en->language = 'en';

    $node_en->uid = 1;

    $node_en->body['und'][0]['value'] = $bodytext_en;
    $node_en->body['und'][0]['summary'] = text_summary(strip_tags($bodytext_en));
    $node_en->body['und'][0]['format'] = 'full_html';

    if ($node_en = node_submit($node_en)) {

      node_save($node_en);
      echo "EN Node with nid " . $node_en->nid . " saved!\n";

    }

    // Set translation
    $node_loaded_tr = node_load($node_tr->nid);
    $node_loaded_tr->tnid = $node_tr->nid;
    node_save($node_loaded_tr);

    $node_loaded_en = node_load($node_en->nid);
    $node_loaded_en->tnid = $node_tr->nid;
    node_save($node_loaded_en);

  } // foreach end $items

}
