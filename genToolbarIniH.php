#!/usr/bin/php
<?php

$header  = "// Auto generated file from toolbar.ini\n\n";
$header .= "const char * TOOLBAR_INI = ";
foreach(file('toolbar.ini') as $l) {
	$l = trim($l);
	$l = str_replace('"', '\\"', $l);
	$header .= "\t\"$l\\n\"\n";
}

$header .= ";\n";

file_put_contents('src/toolbar.ini.h', $header);

?>
