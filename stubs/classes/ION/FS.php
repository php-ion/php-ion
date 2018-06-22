<?php
/*
 * This file is part of PHP-ION.
 *
 * For the full copyright and license information, please view the license.md
 * file that was distributed with this source code.
 */

namespace ION;


use ION\FS\INodeEvent;

class FS {
    const WATCH_MODIFY = 1;

    /**
     * Asynchronously reads the entire contents of a file.
     * This method avoids unnecessary data copies between userland and kernel. If sendfile is available, it uses those
     * functions. Otherwise, it tries to use mmap (or CreateFileMapping on Windows).
     *
     * @param string $filename
     * @param int $offset the offset from which to read data
     * @param int $limit  how much data to read
     *
     * @note do stat() of file
     *
     * @return Deferred
     */
    public static function readFile(string $filename, int $offset = 0, int $limit = -1) : Deferred { }

    /**
     * Watch for changes on filename, where filename is either a file or a directory.
     * Watching files or directories on network file systems (NFS, SMB, etc.) often doesn't work reliably or at all.
     *
     * @param string $filename
     * @param int $events
     *
     * @return Sequence
     */
    public static function watch(string $filename, int $events = 0) : INodeEvent { }
}