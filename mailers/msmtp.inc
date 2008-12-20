<?php

function drupal_mail_wrapper($message) {
  $mailer = _msmtp_get_mailer();
  if (empty($mailer)) return FALSE;

  $mimeheaders = array();
  foreach ($message['headers'] as $name => $value) {
    $mimeheaders[] = $name .': '. mime_header_encode($value);
  }

  $headers = array(
    'To: '. $message['to'],
    'Subject: '. mime_header_encode($message['subject']),
  );

  // For headers, PHP's API suggests that we use CRLF normally,
  // but some MTAs incorrecly replace LF with CRLF. See #234403.
  $header = join("\n", array_merge($mimeheaders, $headers)); 

  // Note: e-mail uses CRLF for line-endings, but PHP's API requires LF.
  // They will appear correctly in the actual e-mail that is sent.
  $body = str_replace("\r", '', $message['body']);

  $result = fwrite($mailer, $header ."\n\n". $body);
  pclose($mailer);

  return TRUE;
}

function _msmtp_get_mailer() {
  $command = variable_get('msmtp_bin');
  $config = variable_get('msmtp_rc');
  $account = variable_get('msmtp_account');

  if (empty($command)
    || empty($config)
    || empty($account))
    return FALSE;

  return popen("$command -t -C $config -a $account", 'w');
}

function mailer_admin_configure(&$form_state) {
  $form = array();
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to msmtp'),
    '#default_value' => variable_get('msmtp_bin', '/usr/bin/msmtp'),
    '#description' => t('Path to the msmtp binary.'),
    '#required' => TRUE,
  );

  $form['rc'] = array(
    '#type' => 'textfield',
    '#title' => t('Configuration file'),
    '#default_value' => variable_get('msmtp_rc', '~/.msmtprc'),
    '#description' => t('Path to the msmtp configuration file.'),
    '#required' => TRUE,
  );

  $form['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Account to use'),
    '#default_value' => variable_get('msmtp_account', 'default'),
    '#description' => t('Use this account from the configuration.'),
    '#required' => TRUE,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

  return $form;
}

function mailer_admin_configure_validate($form, &$form_state) {
  if (!is_executable($form_state['values']['path'])) {
    form_set_error('path', t('Path to msmtp binary is not executable or does not exist.'));
  }

  if (!is_readable($form_state['values']['rc'])) {
    form_set_error('rc', t('Path to configuration file is not readable.'));
  }

  if (!preg_match('/^\w+$/', $form_state['values']['account'])) {
    form_set_error('account', t('Account can only contain alphanumeric characters and no spaces.'));
  }

}

function mailer_admin_configure_submit($form, &$form_state) {
  variable_set('msmtp_bin', $form_state['values']['path']);
  variable_set('msmtp_rc', $form_state['values']['rc']);
  variable_set('msmtp_account', $form_state['values']['account']);

  drupal_set_message(t('Successfully configured msmtp mailer.'));
}

function mailer_reset_configuration() {
  variable_del('msmtp_bin');
  variable_del('msmtp_rc');
  variable_del('msmtp_account');
}
