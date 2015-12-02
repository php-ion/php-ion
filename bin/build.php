#!/usr/bin/env php
<?php

ini_set('memory_limit', -1);

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
		"lcov"     => 'lcov',
	];

    public $shorts = [
        'system' => 'I',
        'diagnostic' => ''

    ];

	public $opts = [];

    public $zend_alloc = 1;

    public function __construct() {
	    foreach($this->binaries as $name => $path) {
			if(getenv(strtoupper("ion_{$name}_exec"))) {
				$this->binaries[$name] = getenv(strtoupper("ion_{$name}_exec"));
			}
	    }
        set_exception_handler(function (\Throwable $exception) {
            $this->line(get_class($exception).": ".$exception->getMessage()." in ".$exception->getFile().":".$exception->getLine()."\n".$exception->getTraceAsString()."\n");
	        exit(1);
        });
    }

	public function getBin(string $name, array $env = []) {
        if($env) {
            foreach($env as $var => &$value) {
                $value = $var."=".$value;
            }
            $env = implode(" ", $env)." ";
        } else {
            $env = "";
        }
		if(isset($this->binaries[$name])) {
			return $env.$this->binaries[$name];
		} else {
			return $env.$name;
		}
	}


	public function write(string $msg) {
		fwrite(STDERR, $msg);
		return $this;
	}

	public function line(string $msg) {
		return $this->write($msg."\n");
	}

	public function hasOption(string $long, string $short = "") {
		$options = getopt($short, [$long]);
		return isset($this->opts[ $long ]) || isset($options[ $long ]) || isset($options[ $short ]);
	}

	public function setOption(string $long, string $value = "") {
		$this->opts[$long] = $value;
		return $this;
	}

	public function getOption(string $long, string $short = "", $default = null) {
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

		if($this->hasOption("build", "b")) {
			$this->setOption("phpize");
			$this->setOption("make");
			$this->setOption("clean");
		}

		if($this->hasOption("setup", "B")) {
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
            if($this->hasOption('coverage', 'o')) {
                $this->exec('./configure --with-ion --enable-ion-debug --enable-ion-coverage', "src/");
            } else {
//                $this->exec('./configure --with-ion='.__DIR__.'/../Libevent --enable-ion-debug', "src/");
                $this->exec('./configure --with-ion --enable-ion-debug', "src/");
            }
		}

		if($this->hasOption('clean', 'c')) {
			$this->exec($this->getBin('make').' clean', "src/");
		}

		if($this->hasOption('make', 'm')) {
			$this->exec($this->getBin('make').' -j2',  "src/");
		}

		if($this->hasOption('info', 'i')) {
			$this->exec($this->getBin('php'/*, ['USE_ZEND_ALLOC' => $this->zend_alloc]*/) . ' -e -dextension=./src/modules/ion.so '.__FILE__." --diagnostic", false, $use_gdb);
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
			$phpunit = $this->getBin('php'/*, ['USE_ZEND_ALLOC' => $this->zend_alloc]*/)." -e -dextension=./src/modules/ion.so ".$this->getBin('phpunit')." --debug --colors=never $group ".$this->getOption('test', 't', '');
			$this->exec($phpunit, false, $use_gdb);
            if($this->hasOption('coverage', 'o')) {
                $this->exec($this->getBin('lcov')." --directory . --capture --output-file coverage.info");
                $this->exec($this->getBin('lcov')." --remove coverage.info 'src/externals/*' '/usr/*' '/opt/*' '*/Zend/*' --output-file coverage.info");
                $this->exec($this->getBin('lcov')." --list coverage.info");
            }
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
            $info[] = $this->_scanFunction($function);
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
	        foreach($class->getProperties() as $prop_name => $prop) {
		        /** @var ReflectionProperty $prop */
		        $mods = implode(' ', Reflection::getModifierNames($prop->getModifiers()));
		        if($prop->class !== $class->name) {
			        $info[] = "  prop $mods {$prop->class}::\${$prop->name}";
		        } else {
			        $info[] = "  prop $mods \${$prop->name}";
		        }

	        }
            foreach($class->getMethods() as $method) {
                $info[] = $this->_scanFunction($method, $class->name);
            }

            $info[] = "}";
        }
        echo implode("\n", $info)."\n";
    }

    /**
     * @param ReflectionFunctionAbstract $function
     * @return string
     */
    private function _scanFunction(ReflectionFunctionAbstract $function, $class_name = "") {
        $params = [];
        foreach($function->getParameters() as $param) {
            /* @var ReflectionParameter $param */
            $type = "";
            $param_name = "$".$param->name;
            if($param->getClass()) {
                $type = $param->getClass()->name;
            } elseif ($param->hasType()) {
                $type = $param->getType();
            } elseif ($param->isArray()) {
                $type = "Array";
            } elseif ($param->isCallable()) {
                $type = "Callable";
            }
            if($param->isVariadic()) {
                $param_name = "...".$param_name;
            }
            if($type) {
                $param_name = $type." ".$param_name;
            }
            if($param->isOptional()) {
                $params[] = "[ ".$param_name." ]";
            } else {
                $params[] = $param_name;
            }
        }
        if($function->hasReturnType()) {
            $return = " : ".$function->getReturnType();
        } else {
            $return = "";
        }
        $declare = $function->name;
        if($function instanceof ReflectionFunction) {
            $declare = "function {$function->name}";
        } elseif ($function instanceof ReflectionMethod) {
	        $mods =  implode(' ', Reflection::getModifierNames($function->getModifiers()));
	        if($function->class !== $class_name) {
		        $declare = "  method {$mods} {$function->class}::{$function->name}";
	        } else {
		        $declare = "  method {$mods} {$function->name}";
	        }

        }
        return "{$declare}(".implode(", ", $params).")$return";
    }

	public function li($name, $value) {
		$this->line(trim($name).": ".trim($value));
	}

    public function printSystemInfo() {
		$this->li("/proc/sys/kernel/core_pattern", `cat /proc/sys/kernel/core_pattern`);
		$this->li("phpunit", `which phpunit`);
		$this->li("composer", `which composer`);
		$this->li("core size", `ulimit -c`);
		$this->li("max fds", `ulimit -n`);
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
			$run_cmd = $this->getBin('gdb').' -ex "handle SIGHUP nostop" -ex "run" -ex "thread apply all bt" -ex "set pagination 0" -batch -return-child-result -silent --args  '.$cmd;
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
  --coverage -o   - generate code coverage information
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


