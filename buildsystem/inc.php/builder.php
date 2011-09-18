<?php

/**
 * This is the buildclass
 */

class Builder {
	private $config;
	private $sourceParser;

	public function __construct($config, $sourceParser) {
		$this->config = $config;
		$this->sourceParser = $sourceParser;
	}

	private function parseDepFile($file) {
		$c = file_get_contents($file);
		
		$c = str_replace("\\\n", '', $c);
		
		$pos = strpos($c, ': ');
		
		$c = substr($c, $pos + 2);
		
		
		$list = preg_split('/ /', $c);
		
		$filelist = array();
		
		foreach($list as $id => $file) {
			$file = trim($file);

			if($file == '') {
				unset($list[$id]);
			} else {
				$rp = realpath($file);
				
				if($rp == "") {
					print "No realpath \"$file\"\n";
				}
				
				$filelist[$rp] = $this->filectime($rp);
			}
		}
		
		return $filelist;
	}
	
	private function filectime($path) {
		$time = filectime($path);
		if($time === false) {
			print "filectime return false for \"$path\"\n";
		}
		return $time;
	}
	
	private function checkActual($list) {
		foreach($list as $file => $time) {
			if($file == 0) {
				continue;
			}
			
			print "checking file $file: $time\n";
			
			$currentTime = $this->filectime($file);
			if($time != $currentTime) {
				print "File {$file} changed\n";
				return false;
			}
		}
		
		return true;
	}
	
	private function buildFile($src, $obj, $flags) {
		$cfg = $this->config->getConfig();
		$compiler = $cfg['COMPILER'];

		if(file_exists($obj . '.ser')) {
			$list = unserialize(file_get_contents($obj . '.ser'));
			if($this->checkActual($list)) {
				print "Skipping {$src}, all files up to date!\n";
				return true;
			}
		}
		
		$dir = dirname($obj);
		if(!file_exists($dir)) {
			mkdir($dir, 0777, true);
		}

		$cmd = "$compiler \"{$src}\" -MM -c -o \"{$obj}.dep\" {$flags}";
		passthru($cmd);
		
		$list = $this->parseDepFile($obj . '.dep');
		
		file_put_contents($obj . '.ser', serialize($list));

		$cmd = "$compiler \"{$src}\" -c -o \"{$obj}.o\" {$flags}";
		
		print $cmd . "\n";
		$returnVar = 0;
		passthru($cmd, $returnVar);

		print "\n";

		if($returnVar == 0) {
			return true;
		} else {
			return false;
		}
	}

	public function build($srcFolder, $objFolder, $target, $config) {
		$ok = true;

		foreach($this->sourceParser->getData() as $gname => $group) {
			foreach($group as $id => $file) {
				if(!file_exists($srcFolder . $file)) {
					$ok = false;
					print "Missing file: $file\n";
				}

			}
		}

		if(!$ok) {
			return false;
		}

		if(!file_exists($objFolder)) {
			mkdir($objFolder, 0777, true);
		}

		//g++ -DHAVE_CONFIG_H -I. -I../../src -I.. -DPACKAGE_DATA_DIR=\""/usr/local/share"\" -DPACKAGE_LOCALE_DIR=\""/usr/local/share/locale"\" -pthread -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pixman-1 -I/usr/include/poppler/glib -I/usr/include/poppler -I/usr/include/libglade-2.0 -I/usr/include/libxml2 -I/usr/include/librsvg-2.0     -I../../src/util -g -rdynamic -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable  -g -O2 -MT xournalpp-ImagesDialog.o -MD -MP -MF .deps/xournalpp-ImagesDialog.Tpo -c -o xournalpp-ImagesDialog.o `test -f 'gui/dialog/ImagesDialog.cpp'


		$libs = $this->config->getLibs();
		$liblist = implode($libs, ' ');
		

		$flags = "-I{$srcFolder} -I{$srcFolder}/util -Wall -g -O2 "
		."-DPACKAGE_DATA_DIR=\\\"\"/usr/local/share\"\\\" "
		."-DPACKAGE_LOCALE_DIR=\\\"\"/usr/local/share/locale\"\\\" "
		."-rdynamic -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable "
		."`pkg-config --cflags {$liblist}`";

		foreach($this->sourceParser->getData() as $gname => $group) {
			foreach($group as $id => $file) {
				if(!$this->buildFile(realpath($srcFolder . $file), $objFolder . $file, $flags)) {
					$ok = false;
				}
			}
		}

		//var_dump($this->config);
	}
}
