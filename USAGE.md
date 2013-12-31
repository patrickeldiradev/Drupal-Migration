# Using the Drupal Migration Package #

## Using Drush Module ##

### Installation ##

* Put _"drushrc.php"_ and _"metu.drush.php"_ to your local Drush module (ex. __/home/user/.drush__ in Linux)
* Change directory to your Drupal site root (ex. __cd ~/wwwhome/drupal/sites/default__ )
* Run _"drush cc all"_ inside your Drupal site root
* You are done!

### Commands ###

You can use following commands by installing the Drush module:

* _drush convert-fs {lang}_ where {lang} is "en" or "tr" (processes "featured slider" nodes and splits fields into ones that can be transferable)
* _drush load-node {nid}_ where {nid} is Drupal Node ID (shows variables in the node, i.e. $node object)
* _drush convert-vc {lang}_ where {lang} is "en" or "tr" (processes "video content" nodes and splits fields into ones that can be transferable)

## Using metu_migrate Module ##

### Installation ###

* Install required modules discussed in [README](README.md) file, under __Requirements__ section
* Enable the modules using admin panel.
* __NOTE:__ You can also use Drush to download and enable the required modules. If you want to use Drush:
    - Change directory to Drupal sites directory (ex. __cd ~/wwwhome/drupal/sites/default__ )
    - Using _"drush dl migrate"_ and _"drush dl migrate_d2d"_ commands, download the required modules into custom modules directory (i.e. __sites/all/modules__)
    - Then, go to Drupal site root (ex. __cd ~/wwwhome/drupal/sites/default__ )
    - Run _"drush en migrate"_ , _"drush en migrate_ui"_ and _"drush en migrate_d2d"_ inside Drupal site root.
* After installing the required modules, put _"metu_migrate"_ directory inside site modules diretory (ex. __sites/default/modules__)
* Enable metu_migrate module from admin panel under _"METU-CC"_ category or use Drush to enable it by _"drush en metu_migrate"_ command inside Drupal site root (ex. __sites/default/modules__)
* Run _"drush cc all"_ or _"Admin Panel -> Performance -> Clear All Caches"_ to enable custom migrate_d2d migrations inside metu_migrate module.
    - __NOTE:__ Clearing caches is needed for the migration system to run. It is designed like that by its author. Look _hook_flush_caches()_ implementation in the [module file](metu_migrate/metu_migrate.module)

### Usage ###

It is possible to use this module inside _Migrate UI_ or _Drush_.

* __Migrate UI:__ Admin Panel -> Content -> Migrate Tab
* __Drush:__ _Migrate_ module has several Drush commands. Sticking to the basics:
    - _drush migrate-import --all_ (runs all migrations and transfers all data from D6 to D7)
    - _drush migrate-rollback --all_ (removes all transferred data from D7 but not from D6)
    - __NOTE:__ You can use _"drush | grep migrate"_ command in Linux to see all Migrate module related Drush commands

## Frequently Asked Questions ##

__How can I download the code in Windows?__

* Solution 1:
    - Download and install "[msysgit](http://msysgit.github.io/)".
    - Set up the paths to include _"git"_ command.
    - Change to your desired directory.
    - Use _"git clone https://github.com/orbingol/migration.git"_ command from CLI.
    - You can see the code inside _"migration"_ subdirectory.
* Solution 2:
    * Use a free and good-looking program like "[Atlassian SourceTree](http://www.sourcetreeapp.com/)".
    * SourceTree also works in Mac.

__How can I download the code in Linux?__

* Use _"git"_ package coming from your distro and then clone the repository.