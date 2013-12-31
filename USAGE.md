# Using the Drupal Migration Package #

## Using Drush Module ##

### Installation ##

* Put [drushrc.php](.drush/drushrc.php) and [orb.drush.php](.drush/orb.drush.inc) to your local Drush module. (ex. __/home/user/.drush__ in Linux)
* Change directory to your Drupal site root. (ex. __cd ~/wwwhome/drupal/sites/default__ )
* Run `drush cc all` inside your Drupal site root.
* You are done!

### Commands ###

You can use following commands by installing the Drush module:

* `drush convert-fs {lang}` where `{lang}` is `en` or `tr`. Defaults to `en`. (processes "featured slider" nodes and splits fields into ones that can be transferable)
* `drush load-node {nid}` where `{nid}` is _Drupal Node ID_. (shows variables in the node, i.e. $node object)
* `drush convert-vc {lang}` where `{lang}` is `en` or `tr`. Defaults to `en`. (processes "video content" nodes and splits fields into ones that can be transferable)
* `drush fix-tr` (fixes translation match problem, migrate_d2d does not correctly map *tnid* values)
* `drush del-tax` (removes statically defined taxonomy term from nodes)

## Using metu_migrate Module ##

### Installation ###

* Install required modules discussed in [README](README.md) file, under [Requirements](README.md#requirements) section.
* Enable the modules using admin panel.
* __NOTE:__ You can also use Drush to download and enable the required modules. If you want to use Drush:
    - Change directory to Drupal sites directory. (ex. __cd ~/wwwhome/drupal/sites/default__ )
    - Using `drush dl migrate` and `drush dl migrate_d2d` commands, download the required modules into custom modules directory. (i.e. __sites/all/modules__)
    - Then, go to Drupal site root. (ex. __cd ~/wwwhome/drupal/sites/default__ )
    - Run `drush en migrate` , `drush en migrate_ui` and `drush en migrate_d2d` inside Drupal site root.
* After installing the required modules, put [orb_migrate](orb_migrate/) directory inside site modules directory. (ex. __sites/default/modules__)
* Enable **orb_migrate** module from admin panel under **Development** category or use Drush to enable it by `drush en orb_migrate` command inside Drupal site root. (ex. __sites/default/modules__)
* Run `drush cc all` or *Admin Panel -> Performance -> Clear All Caches* to enable custom migrate_d2d migrations inside metu_migrate module.
    - __NOTE:__ Clearing caches is needed for the migration system to run. It is designed like that by its author. Look *hook_flush_caches()* implementation in the [module file](orb_migrate/orb_migrate.module).

### Usage ###

It is possible to use this module inside _Migrate UI_ or _Drush_.

* __Migrate UI:__ *Admin Panel -> Content -> Migrate* tab
* __Drush:__ _Migrate_ module has several Drush commands. Sticking to the basics:
    - `drush migrate-import --all` (runs all migrations and transfers all data from D6 to D7)
    - `drush migrate-rollback --all` (removes all transferred data from D7 but not from D6)
    - __NOTE:__ You can use `drush | grep migrate` command in Linux to see all Migrate module related Drush commands

## Step by step instructions ##

* Configure Drupal 7 `settings.php` file and allow Drupal 7 to connect Drupal 6 database. Use `source` array element to define Drupal 6 database connection.
* Install all requirements stated above.
* Go to Drupal 6 site root.
* Run `drush convert-fs en` and `drush convert-fs tr`
* Run `drush convert-vc en` and `drush convert-vc tr`
* Go to Drupal 7 site root.
* Run `drush migrate-import --all`
  * If you have problems, you can rollback migration using `drush migrate-rollback --all`
  * If you added some content to Drupal 6 and want to transfer them to Drupal 7, run `drush migrate-update --all`
* Under Drupal 7 site root run
  * `drush fix-tr` to associate translated nodes.
  * `drush del-tax` to remove defined taxonomy term from defined content types. *Note: Definitions are applied manually to the [orb.drush.php](.drush/orb.drush.php) file.*

## Frequently Asked Questions ##

__How can I download the code in Windows?__

* Solution 1:
    - Download and install [msysgit](http://msysgit.github.io/).
    - Set up the paths to include `git` command.
    - Change to your desired directory.
    - Use `git clone https://github.com/orbingol/migration.git` command from CLI.
    - You can see the code inside *migration* subdirectory.
* Solution 2:
    * Use a free and good-looking program like [Atlassian SourceTree](http://www.sourcetreeapp.com/).
    * SourceTree also works in Mac.

__How can I download the code in Linux?__

* Use `git` package coming from your distro and then clone the repository.
