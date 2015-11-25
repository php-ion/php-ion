<?php

namespace ION;


class SSL {
    const METHOD_AUTO   = 0;

    const METHOD_SSLv2  = 1<<1;
    const METHOD_SSLv3  = 1<<2;
    const METHOD_TLSv10 = 1<<3;
    const METHOD_TLSv11 = 1<<4;
    const METHOD_TLSv12 = 1<<5;

    /**
     * @param int $method
     */
    public static function server(int $method = self::METHOD_AUTO) {}

    /**
     * @param int $method
     */
    public static function client(int $method = self::METHOD_AUTO) {}

    /**
     * SSL constructor. Use methods SSL::server() or SSL::client()
     */
    private function __construct() {}

    /**
     * Normally clients and servers will, where possible, transparently make use of RFC4507bis tickets for stateless session resumption.
     * If this option is set this functionality is disabled and tickets will not be used by clients or servers.
     * @return SSL
     */
    public function noTicket() : self {}

    /**
     * Disable/enable TLS compression.
     * Disable TLS compression can help mitigate the CRIME attack vector.
     * @param bool $state
     * @return SSL
     */
    public function compression(bool $state = true) : self {}

    /**
     * Require verification of SSL certificate used.
     * @param bool $state
     * @return SSL
     */
    public function verifyPeer(bool $state = true) : self {}

    /**
     * Abort if the certificate chain is too deep. Set -1 to no verification.
     * @param int $max
     * @return SSL
     */
    public function verifyDepth(int $max = -1) : self {}

    /**
     * Load local certificate from filesystem
     * Local certificate must be a PEM encoded file which contains your certificate and private key.
     * The private key also may be contained in a separate file specified by $local_pk
     * @param string $local_cert path to local certificate file on filesystem.
     * @param string $local_pk path to local private key file on filesystem in case of separate files for certificate and private key.
     * @return SSL
     */
    public function localCert(string $local_cert, string $local_pk = null) : self {}

    /**
     * Passphrase with which your local_cert file was encoded.
     * @param string $phrase
     * @return SSL
     */
    public function passPhrase(string $phrase) : self {}

    /**
     * @param bool $state
     * @return SSL
     */
    public function allowSelfSigned(bool $state = true) : self {}

    /**
     * Location of Certificate Authority file on local filesystem which should be used with the verifyPeer(true) to authenticate the identity of the remote peer.
     * @param string $file existing file
     * @return SSL
     */
    public function cafile(string $file) : self {}

    /**
     * If Certificate Authority file is not specified or if the certificate is not found there,
     * the directory pointed to by capath is searched for a suitable certificate.
     * $path must be a correctly hashed certificate directory.
     * @param string $path
     * @return SSL
     */
    public function capath(string $path) : self {}

    public function ca(string $cafile, string $capath) : self {}
}