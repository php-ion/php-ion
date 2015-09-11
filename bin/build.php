#!/usr/bin/env php
<?php

$builder = new BuildRunner();

$builder->run();

class BuildRunner {

	const SEGEV_CODE = 139;

	public $binaries = [
		"php"      => PHP_BINARY,
		"phpize"   => 'phpize',
		"make"     => 'make',
		"phpunit"  => 'vendor/bin/phpunit',
		"gdb"      => 'gdb',
	];

	public $opts = [];

    public function __construct() {
	    foreach($this->binaries as $name => $path) {
			if(getenv(strtoupper("ion_{$name}_exec"))) {
				$this->binaries[$name] = getenv(strtoupper("ion_{$name}_exec"));
			}
	    }
        set_exception_handler(function (\Exception $exception) {
            $this->line(get_class($exception).": ".$exception->getMessage()." in ".$exception->getFile().":".$exception->getLine()."\n".$exception->getTraceAsString()."\n");
	        exit(1);
        });
    }

	public function getBin($name) {
		if(isset($this->binaries[$name])) {
			return $this->binaries[$name];
		} else {
			throw new InvalidArgumentException("Binary '$name' not registered");
		}
	}


	public function write($msg) {
		fwrite(STDERR, $msg);
		return $this;
	}

	public function line($msg) {
		return $this->write($msg."\n");
	}

	public function hasOption($long, $short = "") {
		$options = getopt($short, [$long]);
		return isset($this->opts[ $long ]) || isset($options[ $long ]) || isset($options[ $short ]);
	}

	public function setOption($long, $value = "") {
		$this->opts[$long] = $value;
		return $this;
	}

	public function getOption($long, $short = "", $default = null) {
		if(isset($this->opts[$long])) {
			return $this->opts[$long];
		}
		$options = getopt($short."::", [$long."::"]);
		if(isset($options[ $long ])) {
			return $options[ $long ];
		} elseif($short && isset($options[ $short ])) {
			return $options[ $short ];
		} else {
			return $default;
		}
	}

	public function run() {
        if($this->hasOption('system', 'I')) {
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

		if($this->hasOption("build")) {
			$this->setOption("phpize");
			$this->setOption("make");
			$this->setOption("clean");
		}

		if($this->hasOption("setup")) {
			$this->setOption("phpize");
			$this->setOption("make");
			$this->setOption("clean");
			$this->setOption("install");
		}

		if($use_gdb = $this->hasOption("use-gdb")) {
			if($this->getOption("use-gdb")) {
				$this->binaries['gdb'] = $this->getOption("use-gdb");
			}
		}

		if($this->hasOption('phpize', 'p')) {
			if($this->hasOption('clean', 'c')) {
				$this->exec($this->getBin('phpize').' --clean', "src/");
			}
			$this->exec($this->getBin('phpize'), "src/");
			$this->exec('./configure --with-ion', "src/");
		}

		if($this->hasOption('clean', 'c')) {
			$this->exec($this->getBin('make').' clean', "src/");
		}

		if($this->hasOption('make', 'm')) {
			$this->exec($this->getBin('make').' -j4',  "src/");
		}

		if($this->hasOption('info', 'i')) {
			$this->exec($this->getBin('php') . ' -e -dextension=./src/modules/ion.so '.__FILE__." --diagnostic", false, $use_gdb);
		}

		if($this->hasOption("dev")) {
			$this->setOption("test");
			$this->setOption("group", "dev");
		}
		if($this->hasOption('test', 't')) {
			$group = $this->getOption('group', 'g');
			if($group) {
				$group = "--group=".$group;
			} else {
				$group = "";
			}
			$phpunit = $this->getBin('php')." -e -dextension=./src/modules/ion.so ".$this->getBin('phpunit')." --colors=never $group ".$this->getOption('test', 't', '');
			$this->exec($phpunit, false, $use_gdb);
		}
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
	        $mods = [];
	        if($class->isFinal()) {
		        $mods[] = "final";
	        }
	        if($class->isInterface()) {
		        $mods[] = "interface";
	        } elseif($class->isTrait()) {
		        $mods[] = "trait";
	        } else {
		        if($class->isAbstract()) {
			        $mods[] = "abstract";
		        }
		        $mods[] = "class";
	        }

            $info[] = implode(' ', $mods)." {$class->name} {";
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

	public function li($name, $value) {
		$this->line(trim($name).": ".trim($value));
	}

    public function printSystemInfo() {
		$this->li("/proc/sys/kernel/core_pattern", `cat /proc/sys/kernel/core_pattern`);
		$this->li("phpunit", `which phpunit`);
		$this->li("composer", `which composer`);
		$this->li("core size", `ulimit -c`);
    }

	public function isLinux() {
		return strtolower(PHP_OS) == "linux";
	}

	public function isMacOS() {
		return strtolower(PHP_OS) == "darwin";
	}

	public function exec($cmd, $cwd = null, $gdb = false) {
		$prev_cwd = null;
		if($cwd) {
			$cwd = realpath($cwd);
			$prev_cwd = getcwd(); // backup cwd
			chdir($cwd);
		}
		$this->line("\n** ".getcwd().": $cmd");
		if($gdb) {
			$run_cmd = $this->getBin('gdb').' -ex "run"  -ex "thread apply all bt" -ex "set pagination 0" -batch -return-child-result -silent --args  '.$cmd;
			$this->line("*** Using gdb: $run_cmd");
		} else {
			$run_cmd = $cmd.' 2>&1';
		}
		passthru($run_cmd, $code);
		if($code) {
			throw new RuntimeException("Command $cmd failed", $code);
		}

		if($prev_cwd) { // rollback cwd
			chdir($prev_cwd);
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
  --setup,   -B   — alias: --build --install
  --info,    -i   - print info about module
  --system,  -I   — print information about system

Testing:
  --test[=TEST_PATH], -t  — run tests, all or only by path
  --group=GROUP_LIST, -g  - only runs tests from the specified group(s). Option --test required
  --dev                   - alias: --test --group=dev
  --use-gdb[=BINARY]      - use gdb for running tests

Default env:
";
		foreach($this->binaries as $name => $path) {
			echo "  ".strtoupper("ion_{$name}_exec")."=".$path."\n";
		}
		echo "\n";
	}
}


