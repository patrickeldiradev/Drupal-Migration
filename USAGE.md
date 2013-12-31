# Using the Drupal Migration Package #

## Using Drush Module ##

### Installation ##

* Put _drushrc.php_ and _metu.drush.php_ to your local Drush module (ex. __/home/user/.drush__ in Linux)
* Change directory to your Drupal site root (ex. __cd ~/wwwhome/drupal/sites/default__ )
* Run _drush cc all_ inside your Drupal site root
* You are done!

### Commands ###

You can use following commands by installing the Drush module:

* _drush convert-fs {lang}_ where {lang} is "en" or "tr" (processes "featured slider" nodes and splits fields into ones that can be transferable)
* _drush load-node {nid}_ where {nid} is Drupal Node ID (shows variables in the node, i.e. $node object)
* _drush convert-vc {lang}_ where {lang} is "en" or "tr" (processes "video content" nodes and splits fields into ones that can be transferable)
