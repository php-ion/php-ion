#!/usr/bin/env php
<?php

$builder = new BuildRunner();

$builder->run();

class BuildRunner {

    public function __construct() {
        set_exception_handler(function ($exception) {
            fwrite(STDERR, get_class($exception).": ".$exception->getMessage()." in ".$exception->getFile().":".$exception->getLine()."\n".$exception->getTraceAsString());
            exit(1);
        });
    }

	public function hasOption($long, $short) {
		$options = getopt($short, [$long]);
		return isset($options[ $long ]) || isset($options[ $short ]);
	}

	public function getOption($long, $short, $default = null) {
		$options = getopt($short."::", [$long."::"]);
		if(isset($options[ $long ])) {
			return $options[ $long ];
		} elseif(isset($options[ $short ])) {
			return $options[ $short ];
		} else {
			return $default;
		}
	}

	public function run() {
		$this->chdir('src');

        if($this->hasOption('system', 's')) {
            $this->printSystemInfo();
        }

        if($this->hasOption('diagnostic', "")) {
            $this->printInfo();
            return;
        }

		if($this->hasOption("help", "h")) {
			$this->help();
			return;
		}

		if($this->hasOption('phpize', 'p')) {
			$this->exec('phpize');
			$this->exec('./configure --with-ion');
		}

		if($this->hasOption('clean', 'c')) {
			$this->exec('make clean');
		}

		if($this->hasOption('make', 'm')) {
			$this->exec('make -j4');
		}

		if($this->hasOption('info', 'i')) {
			$this->exec(PHP_BINARY . ' -dextension=modules/ion.so '.__FILE__." --diagnostic");
		}

		if($this->hasOption('test', 't')) {
			$this->chdir();
			$group = $this->getOption('group', 'g');
			if($group) {
				$group = "--group=".$group;
			} else {
				$group = "";
			}
			$this->exec(PHP_BINARY." -dextension=".dirname(__DIR__)."/src/modules/ion.so vendor/bin/phpunit $group ".$this->getOption('test', 't', ''));
		}

	}

	public function chdir($rel_path = "") {
		chdir(dirname(__DIR__).($rel_path ? "/" : "").$rel_path);
	}


    public function printInfo() {
        $info = [];
        $ion = new \ReflectionExtension('ion');
        $info[] = $ion->info();
        foreach($ion->getINIEntries() as $ini => $value) {
            $info[] = "ini $ini = ".var_export($value, true);
        }
        foreach($ion->getConstants() as $constant => $value) {
            $info[] = "const $constant = ".var_export($value, true);
        }
        foreach($ion->getFunctions() as $function) {
            $params = [];
            foreach($function->getParameters() as $param) {
                if($param->isOptional()) {
                    $params[] = '[ $'.$param->name.' ]';
                } else {
                    $params[] = '$'.$param->name;
                }
            }
            $info[] = "function {$function->name}(".implode(", ", $params).")";
        }
        foreach($ion->getClasses() as $class) {
            $info[] = "class ".implode(' ', Reflection::getModifierNames($class->getModifiers()))." {$class->name} {";
            if($class->getParentClass()) {
                $info[] = "  extends {$class->getParentClass()->name}";
            }
            foreach($class->getInterfaceNames() as $interface) {
                $info[] = "  implements {$interface}";
            }
            foreach($class->getTraitNames() as $trait) {
                $info[] = "  use {$trait}";
            }
            foreach($class->getConstants() as $constant => $value) {
                $info[] = "  const {$class->name}::{$constant} = ".var_export($value, true);
            }
            foreach($class->getMethods() as $method) {
                $params = [];
                foreach($method->getParameters() as $param) {
                    if($param->isOptional()) {
                        $params[] = '[ $'.$param->name.' ]';
                    } else {
                        $params[] = '$'.$param->name;
                    }
                }
                $info[] = "  method ".implode(' ', Reflection::getModifierNames($method->getModifiers()))." {$method->class}::{$method->name}(".implode(", ", $params).")";
            }

            $info[] = "}";
        }
        echo implode("\n", $info)."\n";
    }

    public function printSystemInfo() {

    }

	public function exec($cmd) {
		echo "\n** ".getcwd().": $cmd\n";
		passthru($cmd.' 2>&1', $code);
		if($code) {
			throw new RuntimeException("Command $cmd failed");
		}
	}

	public function help() {
		echo "Usage: ".basename(__FILE__)." OPTIONS\n
Build:
  --help,    -h   — show help
  --clean,   -c   — make clean
  --make,    -m   — make
  --install, -l   — install module
  --phpize,  -p   — phpize and configure project
  --build,   -b   — alias: --phpize --clean --make
  --info,    -i   - print info about module
  --system,  -s   — print information about system

Testing:
  --test[=TEST_PATH], -t  — run tests, all or only by path
  --group=GROUP_LIST, -g  - only runs tests from the specified group(s). Option --test required
  ";
	}
}


