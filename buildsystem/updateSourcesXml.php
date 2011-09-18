<?php

/**
 * Ths file updates sources.xml, adds new files and moves removed file to the section "removed"
 */

include dirname(__FILE__) . '/inc.php/sourceXmlParser.php';

$parser = new SourceParser();
$parser->load();
$parser->checkFiles(dirname(__FILE__) . '/../src/');
$parser->save();

print "sources.xml updated successfully.\n";
