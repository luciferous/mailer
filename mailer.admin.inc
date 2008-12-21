<?php
// $Id$

/**
 * @file
 * Functions for administration of the Mailer module.
 */

/**
 * Theme the mailer list form.
 */
function theme_mailer_admin_overview($form) {
  foreach (element_children($form) as $key) {
    if (empty($form[$key]['configure'])) continue;
    $row = array();
    $row[] = drupal_render($form['mailers'][$key]);
    $row[] = $key;
    if ($form['mailers']) {
      $row[] = drupal_render($form[$key]['configure']);
      $row[] = drupal_render($form[$key]['reset']);
    }
    $rows[] = $row;
    unset($form[$key]);
  }

  $header = array(t('Selected'), t('Name'), array('data' => t('Operations'), 'colspan' => 2));
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

/**
 * A list of all mailers and which one is in use.
 * @return form form to select which mailer to use
 */
function mailer_admin_overview() {
  $mailers = _mailer_get_mailers();
  $current = variable_get('smtp_library', '');
  $name = '';

  $form = array();
  foreach ($mailers as $id => $mailer) {
    $options[$id] = '';
    $form[$mailer->name]['id'] = array('#value' => $id);
    $form[$mailer->name]['configure'] = array(
      '#value' => l(t('configure'), 'admin/settings/mailer/'.  $mailer->name)
    );
    $form[$mailer->name]['reset'] = array(
      '#value' => l(t('reset'), 'admin/settings/mailer/reset/'.  $mailer->name)
    );

    if ($mailer->filename == $current) {
      $name = $mailer->name;
    }
  }

  $form['mailers'] = array(
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => $name,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Select a mailer'),
  );

  return $form;
}

function mailer_admin_overview_submit($form, &$form_state) {
  $mailer = mailer_load($form_state['values']['mailers']);
  if ($mailer) {
    variable_set('smtp_library', $mailer->filename);
    drupal_set_message(t('Mailer successfully selected. Please configure your mailer.'));
  }
  else {
    drupal_set_message(t('Unable to find your mailer. Please check that %filename exists in the mailers directory.', array('%filename' => $form_state['values']['mailers'])));
  }
}

function mailer_admin_reset() {
  $mailer = mailer_load(arg(4));
  if ($mailer) {
    $form['filename'] = array('#type' => 'hidden', '#value' => $mailer->filename);
    $form['mailer'] = array('#type' => 'hidden', '#value' => $mailer->name);
    return confirm_form($form, t('Are you sure you want to reset the configuration for %mailer?', array('%mailer' => $mailer->name)), 'admin/settings/mailer', t('This action cannot be undone.'), t('Reset'), t('Cancel'));
  }
  else {
    drupal_not_found();
  }
}
function mailer_admin_reset_submit($form, &$form_state) {
  require_once($form_state['values']['filename']);
  mailer_reset_configuration();
  drupal_set_message(t('Successfully reset configuration for %mailer.', array('%mailer' => $form_state['values']['mailer'])));
  $form_state['redirect'] = 'admin/settings/mailer';
}

function mailer_admin_test($form, $mailer) {
  return array(
    'email' => array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => t('Email address'),
    ),
    'mailer' => array(
      '#type' => 'hidden',
      '#value' => $mailer->name,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Send email'),
    ),
  );
}

function mailer_admin_test_submit($form, &$form_state) {
  if (drupal_mail('mailer', 'test', $form_state['values']['email'], language_default(), array('!mailer' => $form_state['values']['mailer']))) {
    drupal_set_message(t('Sent a test email to %email.', array('%email' => $form_state['values']['email'])));
  }
  else {
    drupal_set_message(t('An error occurred while sending the email.'), 'warning');
  }
  drupal_goto('admin/settings/mailer');
}

