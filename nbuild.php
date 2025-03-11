#!/usr/bin/php
<?php

/*

  TSL build script
  ulf.astrom@gmail.com

  The default mode of operation is to compile & link a debug version
  of the binary. We will attempt to minimize build time by reusing
  object files. If a .c file is more recently modified than it's .o,
  it will be recompiled. If any .h file has been changed it will
  trigger a full rebuild.

  The script can also be invoked with the following arguments:
  
  [-v]
  Outputs verbose information what's going on. This _must_ be the
  first argument.
  
  clean
  Remove all (!) files in the temporary directory. Also removes backup
  files ending with a ~.

  gensh
  Generates build.sh files intended for the end-user.

  dist <file>
  Creates a gzipped tarball intended for distribution. It will include
  all necessary files (.c files in $code_modules, _all_ .h files, all
  files in $misc_files). The final archive will be called <file>.tar.gz
  
*/

const BIN_NAME = "tsl";
const PRODUCT = "TSL";

/* What compiler should we use? */
const COMPILE_CMD = "gcc";

/* Additional compile options */
const COMPILE_OPT = "-Wshadow -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -std=c99 -pedantic -Wredundant-decls";

/* Additional debug options. */
const DEBUG_OPT = "-ggdb";

/* Program to use for linking. gcc is easier than invoking ld manually. */
const LINK_CMD = "gcc";

/* Additional libraries we need to link with. */
const LINK_OPT = "-lm";

/* Where to store our temporary object files. */
const TEMP_DIR = "TEMP";

/* For build.sh. */
const ESC_NL = " \\\n\t";

/* For internal debugging. */
$verbose = FALSE;

/* Each of these should represent a .c file to be turned into a .o */
$code_modules = array(
  "main",
  "web",
  "dwiminv",
  "mt19937ar",
  "keymap",
  "menuitem",
  "reading",
  "eat",
  "select",
  "bestiary",
  "wingame",
  "inscript",
  "checks",
  "browser",
  "equip",
  "itemtext",
  "itemprop",
  "ffield",
  "gore",
  "craft",
  "identify",
  "facet",
  "find",
  "pushing",
  "burdened",
  "swimming",
  "memory", 
  "doors",
  "potions",
  "sleep",
  "balls",
  "breath",
  "explode",
  "teleport",
  "backstab",
  "poison",
  "wounds",
  "places",
  "attrs",
  "options",
  "rolls",
  "area",
  "modbuild",
  "missile",
  "ability",
  "stuff",
  "input",
  "losegame",
  "message",
  "saveload",
  "tiles",
  "stacks",
  "vweapon",
  "altitude",
  "dungeon",
  "level",
  "help",
  "creature",
  "ui",
  "item",
  "game",
  "player",
  "debug",
  "monster",
  "combat",
  "effect",
  "treasure",
  "actions",
  "magic",
  "unique",
  "traps",
  "ai",
  "inventory",
  "content",
  "shapeshf",
  "elements",
  "fov",
  "rndnames",
  "clipbrd",
  "anim"
);
  
/* In addition to any .h, other files we want */
$misc_files = array(
  "tsl_conf_example",
  "tsl_conf_dvorak",
  "tileset.png",
  "tilerev.png",
  "tiledim.png",
  "smallfont.tga",
  "font.png",
  "fontrev.png",
  "fontdim.png",
// "tileset.bmp",
  "CHANGES.TXT",
  "README.TXT",
  "COPYING_MT.TXT",
  "LICENSE.TXT",
  "nbuild.php",
  "build_console.sh",
  "build_gui.sh"
  );

$target = "console";

/*
  Generates minimal build shellscripts for the end-user.
*/
function generate_sh($output, $this_target)
{
  global $code_modules;
  global $verbose;

  $ret = "";

  $ret .= "#!/bin/sh\n";
  $ret .= "# Auto-generated build script for " . PRODUCT . "\n\n";
  $ret .= "rm " . BIN_NAME . " 2>/dev/null\n\n";

  $ret .= COMPILE_CMD;

  $ret .= get_compile_opts($this_target);

/*  $ret .= ESC_NL . COMPILE_OPT; */
/*  $ret .= ESC_NL . DEBUG_OPT; */

  $local_modules = array_merge($code_modules, get_extra_modules($this_target));

  foreach ($local_modules as $mod)
  {
    $ret .= ESC_NL . $mod . ".c";
  }

  $ret .= ESC_NL;

  $ret .= ESC_NL . LINK_OPT;

  $ret .= get_link_opts($this_target);

  $ret .= ESC_NL . ESC_NL;
  
  $ret .= "-o " . BIN_NAME . "\n";

  $ret .= "\n";
  $ret .= "exit 0";
  $ret .= "\n";
  
  if (file_put_contents($output, $ret) === FALSE)
  {
    print "ERROR: Couldn't save build.sh!\n";
  }
  elseif ($verbose)
  {
    print $output . " created.\n";
  }
  
  return;
}



/*
  Removes all files in the temp directory, even those not created by this script.
*/
function clean_temp()
{
  global $verbose;
  
  if (is_dir(TEMP_DIR))
  {
    $files = scandir(TEMP_DIR);

    foreach($files as $f)
    {
      if ($f !== "." &&
	  $f !== ".." &&
	  is_file(TEMP_DIR . "/" . $f))
      {
	if (unlink(TEMP_DIR . "/" . $f))
	{
	  if ($verbose)
	    print TEMP_DIR . "/" . $f . " removed.\n";
	}
	else
	  print "ERROR: couldn't delete " . TEMP_DIR . "/" . $f . "\n";
      }
    }
  }

  $files = scandir("./");
  
  foreach($files as $f)
  {
    if ($f !== "." &&
	$f !== ".." &&
	is_file($f) &&
	substr($f, -1) == "~")
    {
      if (unlink($f))
      {
	if ($verbose)
	  print "$f removed.\n";
      }
      else
	print "ERROR: couldn't delete $f\n";
    }
  }

  if ($verbose)
    print "Done cleaning!\n";
  
  return;
}



/*
  Returns all .h files in the current directory.
*/
function get_all_headers()
{
  global $verbose;

  $ret = array();

  $files = scandir("./");
  
  foreach ($files as $file)
  {
    if (substr($file, -2) == ".h")
      array_push($ret, $file);
  }

  if ($verbose)
  {
    print "Found headers:\n";
    print_r($ret);
  }
  
  return $ret;
}



/*
  Compile & link a debug binary.
*/
function build_modules($this_target)
{
  global $code_modules;
  global $verbose;
  global $extra_link;
  global $extra_compile;

  $failed = FALSE;
  $rebuild_all = FALSE;
  $obj_to_link = array();
  $obj_mtime = array();
  $latest_obj_mtime = 0;

  $local_modules = array_merge($code_modules, get_extra_modules($this_target));

  /* Try to set up a temporary work directory for object files. */
  if (is_dir(TEMP_DIR) === FALSE)
  {
    if ($verbose)
      print "Setting up temporary directory " . TEMP_DIR . "...\n";
    
    if (@mkdir(TEMP_DIR) === FALSE)
    {
      print "ERROR: Couldn't create \"" . TEMP_DIR . "\" temporary directory, sorry.\n";
      exit;
    }
  }

  /*
    Set up a list of object files modification time and find the one
    most recently compiled.
  */
  foreach ($local_modules as $mod)
  {
    $obj_file = TEMP_DIR . "/" . $mod . ".o";
    $obj_mtime[$obj_file] = @filemtime($obj_file);

    if ($obj_mtime[$obj_file] > $latest_obj_mtime)
      $latest_obj_mtime = $obj_mtime[$obj_file];
  }

  /*
    Check all header files and if any of them has changed more
    recently than the object files.
  */
  $headers = get_all_headers();

  foreach ($headers as $header)
  {
    if (@filemtime($header) > $latest_obj_mtime)
    {
      print $header . " has changed, rebuilding all.\n";
      $rebuild_all = TRUE;
      break;
    }
  }

  /* Compile each source file into an object file. */
  foreach ($local_modules as $mod)
  {
    $src_file = $mod . ".c"; /* input */
    $obj_file = TEMP_DIR . "/" . $mod . ".o"; /* output */
    $rebuild_this = FALSE;

    /* This file should be included in the final linking */
    array_push($obj_to_link, $obj_file);

    /* Check if this file has been changed since last build */
    $src_time = @filemtime($src_file);

    if ($src_time > $obj_mtime[$obj_file])
    {
      /* Source is more recent than object file, rebuild it. */
      if ($rebuild_all == FALSE)
	print $src_file . " has changed, rebuilding...\n";

      $rebuild_this = TRUE;
    }
     
    if ($rebuild_all || $rebuild_this)
    {
      $cmd =
	COMPILE_CMD . " " . COMPILE_OPT . " " . DEBUG_OPT . " " . get_compile_opts($this_target) . " " . 
	$src_file . " -c -o " . $obj_file;
      
      @unlink($obj_file);

      if ($verbose)
	print $cmd . "\n";

      system($cmd, $ret);

      if ($ret != 0)
	$failed = TRUE;
    }
    elseif ($verbose)
    {
      print $src_file . " is up to date.\n";
    }
  }

  /* Let's try linking, but only if all modules have been properly built. */

  if ($failed === FALSE)
  {
    if (file_exists(BIN_NAME))
    {
      if ((is_file(BIN_NAME) === FALSE) ||
	  (unlink(BIN_NAME) === FALSE))
      {
	print "ERROR: Couldn't delete target file, sorry.\n";
	exit;
      }
    }

    $cmd =
      LINK_CMD . " " . LINK_OPT . " " . get_link_opts($this_target) . " " .
      implode(" ", $obj_to_link) . " " . $extra_link . " -o " . "tsl";

    if ($verbose)
      print $cmd . "\n";

    system($cmd, $ret);
    
    if ($ret != 0)
      $failed = TRUE;
  }
  
  if ($failed)
  {
    print "\nThere were errors.\n";
    exit;
  }
  else
  {
    print "\nBuild successful!\n";
  }

  return;
}


function build_dist($archive)
{
  global $code_modules;
  global $misc_files;
  global $verbose;

  if (strpos($archive, "/"))
  {
    print "ERROR: $archive contains a slash. That's not good.\n";
    exit;
  } 

  if (file_exists($archive))
  {
    print "ERROR: $archive already exists.\n";
    exit;
  }

  /* Check for an existing tar archive, remove if possible */
  $tar = $archive . ".tar";
 
  if (file_exists($tar))
  {
    if (unlink($tar) === FALSE)
    {
      print "ERROR: Couldn't remove $tar.\n";
      exit;
    }
    else if ($verbose)
      print "Removed $tar\n";
  }

  /* Check for an existing tar.gz, remove if possible */
  $targz = $archive . ".tar.gz";

  if (file_exists($targz))
  {
    if (unlink($targz) === FALSE)
    {
      print "ERROR: Couldn't remove $targz.\n";
      exit;
    }
    else if ($verbose)
      print "Removed $targz\n";
  }

  /*
    Check these files to see if we've updated the version string
    everywhere we need to.
  */
  $lines = file("README.TXT");
  print "First line of README.TXT: " . $lines[0];

  $lines = file("main.h");
  foreach ($lines as $line)
  {
    if (strpos($line, "#define TSL_VERSION") !== FALSE)
      print "main.h: $line";
  }

  $lines = file("CHANGES.TXT");
  print "First line of CHANGES.TXT: " . $lines[0];

  print "Building distribution archive $archive...\n";

  $all_files = array_merge($misc_files, get_all_headers());

  foreach ($code_modules as $mod)
    array_push($all_files, $mod . ".c");

  foreach (get_extra_modules("console") as $mod)
    array_push($all_files, $mod . ".c");

  foreach (get_extra_modules("gui") as $mod)
    array_push($all_files, $mod . ".c");

  generate_sh("build_console.sh", "console");
  generate_sh("build_gui.sh", "gui");

  if (mkdir($archive) === FALSE)
  {
    print "ERROR: Couldn't mkdir $archive!\n";
    exit;
  }

  foreach ($all_files as $file)
  {
    if ($verbose)
      print "Copying $file...\n";

    if (copy($file, $archive . "/" . $file) === FALSE)
    {
      print "ERROR: Couldn't copy $file!\n";
      exit;
    }

    if (substr($file, -4) == ".php" ||
	substr($file, -3) == ".sh")
    {
      if ($verbose)
	print "chmod $archive/$file...\n";
      
      chmod($archive . "/" . $file, 0744);
    }
  }

  $cmd = "tar -cvvf \"$tar\" \"$archive\"";

  if ($verbose)
    print "Running $cmd\n";
  
  exec($cmd, $output, $ret);

  if ($ret !== 0)
  {
    print "ERROR: tar failed, sorry.\n";
    exit;
  }

  $cmd = "gzip \"$tar\"";
  
  if ($verbose)
    print "Running $cmd\n";
  
  exec($cmd, $output, $ret);

  if ($ret !== 0)
  {
    print "ERROR: gzip failed, sorry.\n";
    exit;
  }

  print "Distribution \"$targz\" is ready.\n";

  return;  
}



function get_link_opts($target)
{
  if ($target == "console")
    return " -lcurses";
  else if ($target == "gui")
    return " -lallegro -lallegro_image -lallegro_font";
}


function get_compile_opts($target)
{
  if ($target == "console")
    return " -DTSL_CONSOLE ";
  else if ($target == "gui")
    return " -DTSL_GUI ";
}

function get_extra_modules($target)
{
  if ($target == "console")
    return array("console", "glyph");
  else if ($target == "gui")
    return array("allui", "glyph");
}


/* What behaviour do we want? */

$use_arg = 1;

if (isset($argv[1]) && ($argv[1] == "-v"))
{
  $verbose = TRUE;
  $use_arg++;
}

if (isset($argv[$use_arg]))
{
  if ($argv[$use_arg] == "gui" || $argv[$use_arg] == "console")
  {		
    $target = $argv[$use_arg];
    $use_arg++;
  }
}

if (isset($argv[$use_arg]))
{
  if ($argv[$use_arg] == "gensh")
  {
    generate_sh("build_console.sh", "console");
    generate_sh("build_gui.sh", "gui");
    exit;
  }
  else if ($argv[$use_arg] == "clean")
  {
    clean_temp();
  }
  else if ($argv[$use_arg] == "dist")
  {
    if (!isset($argv[$use_arg + 1]))
    {
      print "Usage: dist <file> (without .tar.gz)\n";
      exit;
    }
  
    build_dist($argv[$use_arg + 1]);
    exit;
  }
  else if (isset($argv[$use_arg]))
  {
    print "ERROR: Unknown operation \"" . $argv[$use_arg] . "\". Sorry.\n";
  }

  exit;
}
else
{
  build_modules($target);
}

?>
