<?php
// $Id$

/**
 * @file
 * Used by the Mailer module to send mail via mail() function.
 */

/**
 * Send an e-mail message using mail().
 */
function drupal_mail_wrapper($message) {
  $mimeheaders = array();
  foreach ($message['headers'] as $name => $value) {
    $mimeheaders[] = $name .': '. mime_header_encode($value);
  }
  return mail(
    $message['to'],
    mime_header_encode($message['subject']),
    // Note: e-mail uses CRLF for line-endings, but PHP's API requires LF.
    // They will appear correctly in the actual e-mail that is sent.
    str_replace("\r", '', $message['body']),
    // For headers, PHP's API suggests that we use CRLF normally,
    // but some MTAs incorrecly replace LF with CRLF. See #234403.
    join("\n", $mimeheaders)
  );
}

/**
 * A form allowing Mailer to provide driver-specific configuration.
 */
function mailer_admin_configure() {
  drupal_set_message('Nothing to configure for the default mailer.');
  drupal_goto('admin/settings/mailer');
}

/**
 * Resets or deletes any configuration saved to the database by this
 * driver. 
 */
function mailer_reset_configuration() {
  variable_set('smtp_library', '');
}
