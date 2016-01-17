<?php

namespace ION\HTTP;
use ION\Deferred;

/**
 * Representation of an outgoing, server-side response.
 *
 * Per the HTTP specification, this interface includes properties for
 * each of the following:
 *
 * - Protocol version
 * - Status code and reason phrase
 * - Headers
 * - Message body
 *
 * Responses are considered immutable; all methods that might change state MUST
 * be implemented such that they retain the internal state of the current
 * message and return an instance that contains the changed state.
 */
class Response extends Message {

    public static function parse(string $response) : self { }

    /**
     * Gets the response status code.
     *
     * The status code is a 3-digit integer result code of the server's attempt
     * to understand and satisfy the request.
     *
     * @return int Status code.
     */
    public function getStatusCode() : int { }

    /**
     * Return an instance with the specified status code and, optionally, reason phrase.
     *
     * @link http://tools.ietf.org/html/rfc7231#section-6
     * @link http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
     *
     * @param int $code             The 3-digit integer result code to set.
     * @param string $reason_phrase The reason phrase to use with the
     *                              provided status code; if none is provided, implementations MAY
     *                              use the defaults as suggested in the HTTP specification.
     *
     * @return self
     * @throws \InvalidArgumentException For invalid status code arguments.
     */
    public function withStatus(int $code, string $reason_phrase = '') : self { }

    /**
     * Gets the response reason phrase associated with the status code.
     *
     * @link http://tools.ietf.org/html/rfc7231#section-6
     * @link http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
     *
     * @return string Reason phrase; must return an empty string if none present.
     */
    public function getReasonPhrase() : string { }
}