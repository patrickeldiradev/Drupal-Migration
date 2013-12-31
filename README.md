# Drupal Module for Migration from D6 to D7 #

## Introduction ##
- - -

This repository contains a Drush module and migrate_d2d module for migrating from D6 to D7.

__Drush Module__ : This module is used for converting some several fields to another separate fields in a content type. This separation operation makes field-to-field migration easy.
__Migrate Module__ : This module is the main migration module.

This module is capable of doing

* Content type
* Taxonomy
* File
* User

migrations while processing dependencies between these elements.

Content types with related fields and taxonomy vocabularies should be created before running these migrations but there is no need to create vocabulary terms inside a taxonomy because, migration automatically does term transfer.


## Requirements ##
- - -

### For Drupal 6 ###

* Date
* Calendar (optional)

### For Drupal 7 ###

* Migrate
* Drupal-to-Drupal Migration (migrate_d2d)


## Notes ##
- - -

### Module Changes ###

* SWFUpload -> jQuery File Upload (https://drupal.org/project/jquery_file_upload)

### Problems & Solutions ###

* When Date Popup is used, related field does not updated with correct data from the database. Probably, this is a problem of local testing environment.
* After deleting some content types, SQL query changes and Drush module should be updated to fit these changes.
