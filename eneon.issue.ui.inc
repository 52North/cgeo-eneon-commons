<?php
/*
  * Copyright (C) 2016
  *
  * This program is free software; you can redistribute it and/or modify it
  * under the terms of the GNU General Public License version 2 as published
  * by the Free Software Foundation.
  *
  * If the program is linked with libraries which are licensed under one of
  * the following licenses, the combination of the program with the linked
  * library is not considered a "derivative work" of the program:
  *
  *     - Apache License, version 2.0
  *     - Apache Software License, version 1.0
  *     - GNU Lesser General Public License, version 3
  *     - Mozilla Public License, versions 1.0, 1.1 and 2.0
  *     - Common Development and Distribution License (CDDL), version 1.0
  *
  * Therefore the distribution of the program linked with libraries licensed
  * under the aforementioned licenses, is permitted by the copyright holders
  * if the distribution is compliant with both the GNU General Public
  * License version 2 and the aforementioned licenses.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details.
  */

require_once 'includes/eneon.common.inc';

/**
 * Implements hook_form()
 */
function eneon_issue_form($form, &$form_state, $entity) {

  $form_state[ENEON_ENTITY_ISSUE] = $entity;
  $wrapper = entity_metadata_wrapper(ENEON_ENTITY_ISSUE, $entity);

  // https://www.drupal.org/docs/7/api/javascript-api/managing-javascript-in-drupal-7
  $form['#attached']['js'] = array(
      drupal_get_path('module', 'eneon') . '/js/suggest_networks.js',
  );
  $form['title'] = array(
      '#title' => t('Title'),
      '#type' => 'textfield',
      '#default_value' => $wrapper->title->value(),
      '#required' => TRUE,
  );
  $form['description'] = array(
      '#title' => t('Issue description'),
      '#type' => 'textarea',
      '#default_value' => $wrapper->description->value(),
      '#required' => TRUE,
  );
  $options = array();
  $all_sbas = eneon_get_sbas_like();
  foreach ($all_sbas as $sba) {
    $options[$sba->tid] = $sba->name;
  }
  $form['sbas'] = array (
  		'#title' => t('Social Benefit Areas'),
  		'#type' => 'select',
  		'#multiple' => TRUE,
      '#options' => $options,
  		'#description' => t('This is some full text'),
  		'#required' => TRUE,
  );
  $form['fieldset_eneon_proposed_networks'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proposed Networks'),
  );
  $form['fieldset_eneon_proposed_networks']['networks'] = eneon_get_initial_networks_node($form);
  if (isset($form_state['values'])) {
    if ( !empty($user_input)) {
      // TODO orphaned case?
      eneon_create_ajax_network_suggestions($form, $form_state, $user_input);
    } else {
      $nids = eneon_get_selected_network_nids($form_state);
      eneon_render_networks($form, $nids, eneon_get_initial_networks_node($form));
    }
  }

  field_attach_form(ENEON_ENTITY_ISSUE, $entity, $form, $form_state);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#weight' => 5,
      '#submit' => array('eneon_issue_form_submit'),
  );
  if (empty($entity->is_new)) {
    $form['actions']['delete'] = array(
        '#type' => 'submit',
        '#value' => t('Delete'),
        '#weight' => 15,
        '#submit' => array('eneon_issue_form_submit_delete'),
    );
  }

  $lang = isset($form['description']['#language']) ? $form['description']['#language'] : LANGUAGE_NONE;
  $user_input = eneon_get_user_input($form_state, $lang);
  $settings = array(
      'title' => isset($user_input['title']) ? $user_input['title'] : '',
      'animationEffect' => variable_get('effect', 'none'),
  );
  $form['#attached']['js'][] = array(
      'type' => 'file',
      'data' => drupal_get_path('module', 'eneon') . '/js/suggest_networks.js',
  );
  $form['#attached']['js'][] = array(
      'type' => 'setting',
      'data' => $settings,
  );
  return $form;
}

/**
 * Entity edit form "Save" submit handler.
 */
function eneon_issue_form_submit($form, &$form_state) {
//   $issue = entity_ui_form_submit_build_entity($form, $form_state);
//   $issue->save();
//   $form_state['redirect'] = 'admin/issues';
  $entity = $form_state[ENEON_ENTITY_ISSUE];
  entity_form_submit_build_entity(ENEON_ENTITY_ISSUE, $entity, $form, $form_state);
  $entity->is_new_revision = TRUE;
  $wrapper = entity_metadata_wrapper(ENEON_ENTITY_ISSUE, $entity);
  $wrapper->save();
  drupal_set_message(t('The issue: @title has been saved.', array('@title' => entity_label(ENEON_ENTITY_ISSUE, $entity))));

  // Redirect user to edited entity page.
  $entity_uri = entity_uri(ENEON_ENTITY_ISSUE, $entity);
  $form_state['redirect'] = $entity_uri['path'];
}
/**
 * Entity edit form "Delete" submit handler.
 */
function eneon_issue_form_submit_delete($form, &$form_state) {
  // Redirect user to "Delete" URI for this entity.
  $entity = $form_state[ENEON_ENTITY_ISSUE];
  $entity_uri = entity_uri(ENEON_ENTITY_ISSUE, $entity);
  $form_state['redirect'] = $entity_uri['path'] . '/delete';
}
/**
 * Delete form builder
 */
function eneon_issue_form_delete($form, &$form_state, $entity) {
  $form_state[ENEON_ENTITY_ISSUE] = $entity;
  return confirm_form(
      $form,
      t('Are you sure you want to delete issue @title?', array('@title' => entity_label(ENEON_ENTITY_ISSUE, $entity))),
      entity_uri(ENEON_ENTITY_ISSUE, $entity)['path'],
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Entity "Confirm delete" form submit handler.
 */
function eneon_issue_form_delete_submit($form, &$form_state) {
  $entity = $form_state[ENEON_ENTITY_ISSUE];
  $wrapper = entity_metadata_wrapper(ENEON_ENTITY_ISSUE, $entity);
  $wrapper->delete();

  // Redirect user.
  drupal_set_message(t('Issue @title deleted.', array('@title' => entity_label(ENEON_ENTITY_ISSUE, $entity))));
  $form_state['redirect'] = '<front>';
}

function eneon_get_initial_networks_node(&$form) {
  return array(
      '#type' => 'item',
      '#prefix' => '<div id="eneon_proposed_networks" class="description">'
      . t('Will update automatically'),
      '#suffix' => '</div>',
  );
}

function eneon_get_selected_network_nids($form_state) {
  if ( !isset($form_state['values'])) {
    return array();
  }
  $network_checkboxes = eneon_preg_grep_keys("/^network_\d+$/", $form_state['values']);
  $selected = array_filter($network_checkboxes, function($cb) { return $cb == 1; });
  return array_map(function($n) { return eneon_after('network_', $n); }, array_keys($selected));
}

function eneon_after ($this, $inthat) {
  // http://php.net/manual/de/function.substr.php#112707
  if (!is_bool(strpos($inthat, $this)))
    return substr($inthat, strpos($inthat,$this)+strlen($this));
};

function eneon_preg_grep_keys($pattern, $input, $flags = 0) {
  // http://php.net/manual/de/function.preg-grep.php#111673
  return array_intersect_key($input, array_flip(preg_grep($pattern, array_keys($input), $flags)));
}

function eneon_get_user_input($form_state, $lang) {
  $user_input = array();
  if ( !empty($form_state['values']['title'])) {
    $user_input['title'] = $form_state['values']['title'];
  }
  if ( !empty($form_state['values']['description'][$lang][0]['value'])) {
    $user_input['description'] = $form_state['values']['description'][$lang][0]['value'];
  }
  if ( !empty($form_state['values']['field_eneon_issue_tags'][$lang])) {
    $user_input['tags'] = $form_state['values']['field_eneon_issue_tags'][$lang];
  }
  return $user_input;
}

function eneon_propose_networks_callback($form, $form_state) {
  return $form['fieldset_eneon_proposed_networks']['networks'];
}

function eneon_create_ajax_network_suggestions(&$form, $form_state, $user_input = array()) {
  $lang = $form_state['values']['language'];
  $user_input = eneon_get_user_input($form_state, $lang);
  // perform query based on user input
  $query = empty($user_input)
  ? array()
  : db_select('eneon_networks', 'n')
  ->fields('n', array ('id'))
  ->condition(db_or()
      ->condition('n.title', $user_input['title'], 'LIKE')
      ->condition('n.description', $user_input));

  dpm((string)$query);
  dpm($query->arguments());

  $matching_nids = $query
  ->execute()
  ->fetchCol(); // returns an indexed array

  // keep selected networks
  // TODO make user aware of non-matching networks
  $selected_nids = eneon_get_selected_network_nids($form_state);
  $nids = $matching_nids + $selected_nids;

  eneon_render_networks($form, $nids, array(
      '#type' => 'item',
      '#prefix' => '<div id="eneon_proposed_networks" class="description">'
      . t('Sorry, couldn\'t relate any network to your issue!'),
      '#suffix' => '</div>',
  ));
}

function eneon_render_networks(&$form, $nids, $node_empty_case) {

  // TODO rendering should be done via network entity view

  if (empty($nids)) {
    $form['fieldset_eneon_proposed_networks']['networks'] = $node_empty_case;
  } else {
    $nodes = eneon_network_load_multiple($nids);
    $form['fieldset_eneon_proposed_networks']['networks'] = array(
        '#type' => 'item',
        '#prefix' => '<div id="eneon_proposed_networks"><div class="description">'
        . t('Select ENEON networks to be attached to your issue.'),
        '#suffix' => '</div></div>',
        '#weight' => -1,
    );
    foreach ($nodes as $node) {
      $form['fieldset_eneon_proposed_networks']['networks']['network_' . $node->nid] = array(
          '#type' => 'checkbox',
          '#title' => $node->title,
      );
    }
  }
}
