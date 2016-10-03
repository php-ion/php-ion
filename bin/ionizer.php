#!/usr/bin/env php
<?php

ini_set('memory_limit', -1);

$builder = new BuildRunner();

$builder->run();

class BuildRunner {

	const SEGEV_CODE = 139;

    const GDB_NONE   = 0;
    const GDB_LOCAL  = 1;
    const GDB_SERVER = 2;

	public $binaries = [
		"php"       => PHP_BINARY,
		"phpize"    => 'phpize',
		"make"      => 'make',
		"phpunit"   => 'vendor/bin/phpunit',
		"gdb"       => 'gdb',
		"gdbserver" => 'gdbserver',
		"lcov"      => 'lcov',
		"docker"    => 'docker',
	];

    public $options = [
        'diagnostic' => [
            'short' => '',
            'desc'  => 'Prints extension diagnostic info.'
        ],
        'help' => [
            'short' => 'h',
            'desc'  => 'Prints this usage information.'
        ],
        'clean' => [
            'short' => 'c',
            'desc'  => 'Deletes all the already compiled object files.'
        ],
        'make' => [
            'short' => 'm',
            'desc'  => 'Compile ION extension (make required)'
        ],
        'coverage' => [
            'short' => 'o',
            'desc'  => 'Compile extension with code coverage and generate code coverage report in Clover XML format (lcov required).'
        ],
        'install' => [
            'short' => '',
            'desc'  => 'Install extension.'
        ],
        'phpize' => [
            'short' => 'p',
            'desc'  => 'Prepare the build environment for a PHP extension (phpize required).'
        ],
        'build' => [
            'short' => 'b',
            'desc'  => 'Build and compile the extension. Alias of --phpize --clean --make.'
        ],
        'setup' => [
            'short' => 'B',
            'desc'  => 'Build, compile and install the extension. Alias of --build --install.'
        ],
        'info' => [
            'short' => 'i',
            'desc'  => 'Prints info about the extension.'
        ],
        'system' => [
            'short' => 'I',
            'desc'  => 'Prints useful for extension information about this OS.'
        ],
        'test' => [
            'short' => 't',
            'desc'  => 'Run tests (phpunit required).',
            'extra' => "[=PATH]"
        ],
        'group' => [
            'short' => '',
            'desc'  => 'Only runs tests from the specified group(s).',
            'extra' => '=GROUP'
        ],
        'dev' => [
            'short' => '',
            'desc'  => 'Only runs tests from the dev group. Alias of --group=dev.'
        ],
        'gdb' => [
            'short' => '',
            'desc'  => 'Runs tests via GDB (gdb required).'
        ],
        'gdb-server' => [
            'desc'  => 'Runs tests via GDB Server (gdbserver required).',
            'extra' => '=HOST:PORT'
        ],
        'docker-gdb-server' => [
            'desc'  => 'Start docker image with gdb-server (docker required)',
        ],
        'docker-sync' => [
            'desc'  => 'Copy extension files into docker dev directory.',
        ],
        'docker-sync-rebuild' => [
            'desc'  => 'Copy extension files into docker dev directory and rebuild if .m4 or .h files has changes.',
            'extra' => '=HOST:PORT'
        ]
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

    /**
     * Get program binary name/path
     * @param string $name short name
     * @param array $env
     *
     * @return string
     */
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

    /**
     * Write string to stderr
     *
     * @param string $msg
     *
     * @return $this
     */
	public function write(string $msg) {
		fwrite(STDERR, $msg);
		return $this;
	}

    /**
     * Write line to stderr
     *
     * @param string $msg
     *
     * @return BuildRunner
     */
	public function line(string $msg) {
		return $this->write($msg."\n");
	}

    /**
     * @param string $option
     * @param string $key
     * @param string $default
     *
     * @return mixed
     */
    private function getOptionInfo(string $option, string $key = '', $default = '') {
        if(isset($this->options[$option])) {
            if($key) {
                return $this->options[$option][$key] ?? $default;
            } else {
                return $this->options[$option];
            }
        } else {
            throw new LogicException("Option <$option> doesn't exists");
        }
    }

    /**
     * @param string $option
     *
     * @return string
     */
    private function getShort(string $option) {
        return $this->getOptionInfo($option, 'short', '');
    }

    private function getDesc(string $option) {
        return $this->getOptionInfo($option, 'desc', '');
    }

    private function getExtra(string $option) {
        return $this->getOptionInfo($option, 'extra', '');
    }

    /**
     * Checks whether the parameter
     *
     * @param string $long
     *
     * @return bool
     *
     */
	public function hasOption(string $long) {
        $short = $this->getShort($long);
		$options = getopt($short, [$long]);
		return isset($this->opts[ $long ]) || isset($options[ $long ]) || isset($options[ $short ]);
	}

    /**
     * Sets parameter
     * @param string $long
     * @param string $value
     *
     * @return $this
     */
	public function setOption(string $long, string $value = "") {
		$this->opts[$long] = $value;
		return $this;
	}

    /**
     * @param string $long
     * @param mixed $default
     *
     * @return mixed
     */
	public function getOption(string $long, $default = null) {
		if(isset($this->opts[$long])) {
			return $this->opts[$long];
		}
        $short = $this->getShort($long);
		$options = getopt($short."::", [$long."::"]);
		if(isset($options[ $long ])) {
			return $options[ $long ];
		} elseif($short && isset($options[ $short ])) {
			return $options[ $short ];
		} else {
			return $default;
		}
	}

    /**
     * Run dispatcher
     */
	public function run() {
        if($this->hasOption('system')) {
            $this->printSystemInfo();
        }

        if($this->hasOption('diagnostic')) {
            $this->printInfo();
            return;
        }

		if($this->hasOption("help")) {
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

		if($this->hasOption("gdb")) {
            $gdb = self::GDB_LOCAL;
		} elseif($this->hasOption("gdb-server")) {
            $gdb = self::GDB_SERVER;
        } else {
            $gdb = self::GDB_NONE;
        }

		if($this->hasOption('phpize')) {
			if($this->hasOption('clean')) {
				$this->exec($this->getBin('phpize').' --clean', "src/");
			}
			$this->exec($this->getBin('phpize'), "src/");
            if($this->hasOption('coverage')) {
                $this->exec('./configure --with-ion --enable-ion-debug --enable-ion-coverage', "src/");
            } else {
//                $this->exec('./configure --with-ion='.__DIR__.'/../Libevent --enable-ion-debug', "src/");
                $this->exec('./configure --with-ion --enable-ion-debug', "src/");
            }
		}

		if($this->hasOption('clean')) {
			$this->exec($this->getBin('make').' clean', "src/");
		}

		if($this->hasOption('make')) {
			$this->exec($this->getBin('make').' -j2',  "src/");
		}

		if($this->hasOption('info')) {
			$this->exec($this->getBin('php'/*, ['USE_ZEND_ALLOC' => $this->zend_alloc]*/) . ' -e -dextension=./src/modules/ion.so '.__FILE__." --diagnostic", false, $gdb);
		}

		if($this->hasOption("dev")) {
			$this->setOption("test");
			$this->setOption("group", "dev");
		}
		if($this->hasOption('test')) {
			$group = $this->getOption('group');
			if($group) {
				$group = "--group=".$group;
			} else {
				$group = "";
			}
			$phpunit = $this->getBin('php'/*, ['USE_ZEND_ALLOC' => $this->zend_alloc]*/)." -e -n -dextension=./src/modules/ion.so ".$this->getBin('phpunit')." --colors=always $group ".$this->getOption('test', '');
			$this->exec($phpunit, false, $gdb);
            if($this->hasOption('coverage')) {
                $this->exec($this->getBin('lcov')." --directory . --capture --output-file coverage.info");
                $this->exec($this->getBin('lcov')." --remove coverage.info 'src/deps/*' '/usr/*' '/opt/*' '*/Zend/*' --output-file coverage.info");
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
        $this->li("phpunit", `which phpunit`);
        $this->li("composer", `which composer`);
        if(file_exists("/proc/sys/kernel/core_pattern")) {
            $this->li("core_dump.storage", `cat /proc/sys/kernel/core_pattern`);
        } elseif(file_exists("/cores")) {
            $this->li("core_dump.storage", '/cores');
        } else {
            $this->li("core_dump.storage", 'none');
        }
        $this->li("core_dump.size", `ulimit -c`);
		$this->li("ulimit.fd_count", `ulimit -n`);
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
		if($gdb == self::GDB_LOCAL) {
			$run_cmd = $this->getBin('gdb').' -ex "handle SIGHUP nostop SIGCHLD nostop" -ex "run" -ex "thread apply all bt" -ex "set pagination 0" -batch -return-child-result -silent --args  '.$cmd;
			$this->line("*** Using gdb: $run_cmd");
		} elseif($gdb == self::GDB_SERVER) {
            $run_cmd = $this->getBin('gdbserver').' 127.0.0.1:8017 --args  '.$cmd;
            $this->line("*** Using gdbserver: $run_cmd");
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

    public function dockerSync($from, $to) {
        exec("cd $from && git ls-files", $files, $code);
        if($code) {
            throw new RuntimeException("Failed git ls-files");
        }
        foreach($files as $file) {

            copy($from.'/'.$file, $to.'/'.$file);
        }
    }

    /**
     * @param array $options
     * @param int $minimal
     *
     * @return string
     */
    private function compileHelp(array $options, $minimal = 0) {
        $table = [];
        foreach($options as $option) {
            $option_key = "  ";
            if($this->getShort($option)) {
                $option_key .= "-".$this->getShort($option).",";
            } else {
                $option_key .= "   ";
            }
            $option_key .= " --".$option.$this->getExtra($option);

            $table[$option_key] = $this->getDesc($option);
        }

        $size = max($minimal, ... array_map("strlen", array_keys($table)));

        foreach($table as $key => &$row) {
            $row = sprintf("%-{$size}s  %s", $key, $row);
        }

        return implode("\n", $table);
    }

    /**
     *
     */
	public function help() {
		echo "Usage: ".$_SERVER["PHP_SELF"]." [OPTIONS] ...\n
Build:
".$this->compileHelp(["help", "clean", "make", "coverage", "phpize", "build", "install"], 20)."

Information:
".$this->compileHelp(["info", "system"], 20)."

Testing:
".$this->compileHelp(["test", "group", "dev", "gdb", "gdb-server"], 20)."

Docker:
".$this->compileHelp(["docker-gdb-server", "docker-sync", "docker-sync-rebuild"], 20)."

Environment:
";
		foreach($this->binaries as $name => $path) {
			printf("  %-36s  -> %s\n", strtoupper("ion_{$name}_exec")."=".$path, trim(`command -v $path`));
		}
		echo "\n";
	}
}


