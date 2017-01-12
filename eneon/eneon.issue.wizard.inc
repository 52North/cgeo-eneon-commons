<?php

/**
 * @file
 * Extensible wizard form example.
 */

/**
 * Extensible wizard form example.
 *
 * This is an example of a multistep form using a wizard style. It will include
 * the 'Previous' and 'Next' buttons when required, and a 'Finish' button at the
 * last stage of the form submission.
 *
 * This example is an extensible skeleton that can include (even
 * programmatically) more steps. The demonstration form includes three steps,
 * each step having its own validation functions.
 *
 * How to extend this example:
 * - Steps are defined in the _eneon_issue_steps() function. Include or alter
 *   the steps as you require.
 * - For each step, implement the corresponding 'form' function (see
 *   'eneon_issue_wizard_metadata' for the first step in this example.)
 *   Each step is a regular form, and the wizard collects all the values of the
 *   included forms.
 * - Optionally, you may include custom validation functions using the regular
 *   validation hook (formname_validate). The wizard uses these validation
 *   functions for each step.
 * - The most important customization step is to change the submit handler and
 *   do whatever you want with the collected information. In this case, the
 *   example just shows the collected values in the various steps.
 * @ingroup eneon
 */

// skeleton taken from form_example module in Drupal's examples:
//   https://www.drupal.org/project/examples

// TODO remove comments from example

require_once 'eneon.info.inc';
require_once 'includes/eneon.common.inc';

function eneon_form_eneon_issue_node_form_alter(&$form, &$form_state, $form_id) {
  $node = $form_state['node'];
  if (isset($node->nid) && isset($node->type) && $node->type == ENEON_BUNDLE_ISSUE) {
    drupal_goto('/issues/edit/' . $node->nid);
  }
}

/**
 * Returns the list of steps and their associated forms.
 *
 * This has been separated to clarify and easy the understanding of this
 * example. You should edit this function to include the steps your
 * wizard/multistep form requires.
 *
 * @return array
 *   List of steps and their forms.
 *
 * @ingroup eneon
 */
function _eneon_issue_wizard_steps() {
  return array(
    1 => array(
      'form' => 'eneon_issue_wizard_metadata',
    ),
    2 => array(
      'form' => 'eneon_issue_wizard_network_suggestions',
    ),
  );
}

/**
 * The primary formbuilder function for the wizard form.
 *
 * This is the form that you should call with drupal_get_form() from your code,
 * and it will include the rest of the step forms defined. You are not required
 * to change this function, as this will handle all the step actions for you.
 *
 * This form has two defined submit handlers to process the different steps:
 *  - Previous: handles the way to get back one step in the wizard.
 *  - Next:     handles each step form submission,
 *
 * The third handler, the finish button handler, is the default form_submit
 * handler used to process the information.
 *
 * You are not required to change the next or previous handlers, but you must
 * change the eneon_issue_wizard_submit handler to perform the operations you
 * need on the collected information.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard($form, &$form_state, $entity=NULL) {

  $create_new = !isset($form_state[ENEON_BUNDLE_ISSUE]) && !isset($entity);
  $entity = isset($form_state[ENEON_BUNDLE_ISSUE]) ? $form_state[ENEON_BUNDLE_ISSUE] : $entity;
  $entity = is_string($entity) ? node_load($entity) : $entity;
  $form_state[ENEON_BUNDLE_ISSUE] = $create_new
      ? eneon_create_issue()
      : $entity;

  // Initialize a description of the steps for the wizard.
  if (empty($form_state['step'])) {
    $form_state['step'] = 1;

    // This array contains the function to be called at each step to get the
    // relevant form elements. It will also store state information for each
    // step.
    $form_state['step_information'] = _eneon_issue_wizard_steps();
  }
  $step = &$form_state['step'];
  $title = $create_new
      ? t('Create Issue: Step @step', array('@step' => $step))
      : t('Edit Issue: Step @step', array('@step' => $step));
  drupal_set_title($title);

  // Call the function named in $form_state['step_information'] to get the
  // form elements to display for this step.
  $form = $form_state['step_information'][$step]['form']($form, $form_state);

  // Show the 'previous' button if appropriate. Note that #submit is set to
  // a special submit handler, and that we use #limit_validation_errors to
  // skip all complaints about validation when using the back button. The
  // values entered will be discarded, but they will not be validated, which
  // would be annoying in a "back" button.
  if ($step > 1) {
    $form['prev'] = array(
      '#type' => 'submit',
      '#value' => t('Previous'),
      '#name' => 'prev',
      '#submit' => array('eneon_issue_wizard_previous_submit'),
      '#limit_validation_errors' => array(),
    );
  }

  // Show the Next button only if there are more steps defined.
  if ($step < count($form_state['step_information'])) {
    // The Next button should be included on every step.
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#name' => 'next',
      '#submit' => array('eneon_issue_wizard_next_submit'),
    );
  }
  else {
    // Just in case there are no more steps, we use the default submit handler
    // of the form wizard. Call this button Finish, Submit, or whatever you
    // want to show. When this button is clicked, the
    // eneon_issue_wizard_submit handler will be called.
    $form['finish'] = array(
      '#type' => 'submit',
      '#value' => t('Finish'),
    );
  }

  // Include each validation function defined for the different steps.
  if (function_exists($form_state['step_information'][$step]['form'] . '_validate')) {
    $form['next']['#validate'] = array($form_state['step_information'][$step]['form'] . '_validate');
  }

  return $form;
}

/**
 * Submit handler for the "previous" button.
 *
 * This function:
 * - Stores away $form_state['values']
 * - Decrements the step counter
 * - Replaces $form_state['values'] with the values from the previous state.
 * - Forces form rebuild.
 *
 * You are not required to change this function.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard_previous_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['input'];
  if ($current_step > 1) {
    $current_step--;
    $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the 'next' button.
 *
 * This function:
 * - Saves away $form_state['values']
 * - Increments the step count.
 * - Replace $form_state['values'] from the last time we were at this page
 *   or with array() if we haven't been here before.
 * - Force form rebuild.
 *
 * You are not required to change this function.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard_next_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];

  if ($current_step < count($form_state['step_information'])) {
    $current_step++;
    if (!empty($form_state['step_information'][$current_step]['stored_values'])) {
      $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
    }
    else {
      $form_state['values'] = array();
    }
    // Force rebuild with next step.
    $form_state['rebuild'] = TRUE;
    return;
  }
}

/**
 * The previous code was a 'skeleton' of a multistep wizard form. You are not
 * required to change a line on the previous code (apart from defining your own
 * steps in the _eneon_issue_steps() function.
 *
 * All the code included from here is the content of the wizard, the steps of
 * the form.
 *
 * First, let's show the defined steps for the wizard example.
 * @ingroup eneon
 */

/**
 * Returns form elements for the 'issue metadata' page of the wizard.
 *
 * This is the first step of the wizard, asking for: title, description
 * and metainfos (like SBAs, themes) which is taken to suggest networks
 * in the following step.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard_metadata($form, &$form_state) {
  $form = array();

  $entity = $form_state[ENEON_BUNDLE_ISSUE];
  $wrapper = entity_metadata_wrapper('node', $entity);

  $title_value = !empty($form_state['values']['title']) ? $form_state['values']['title'] : $wrapper->title->value();
  $description_value = !empty($form_state['values']['description']) ? $form_state['values']['description'] : $wrapper->{ENEON_ISSUE_DESCRIPTION}->value();
  $selected_themes = !empty($form_state['values']['themes']) ? $form_state['values']['themes'] : array_column($wrapper->{ENEON_ISSUE_SELECTED_THEMES}->value(), 'tid');
  $selected_sbas = !empty($form_state['values']['sbas']) ? $form_state['values']['sbas'] : array_column($wrapper->{ENEON_ISSUE_SELECTED_SBAS}->value(), 'tid');

  $form['title'] = array(
      '#title' => t('Title'),
      '#type' => 'textfield',
      '#default_value' => $title_value,
      '#required' => TRUE,
  );
  $form['description'] = array(
      '#title' => t('Issue description'),
      '#type' => 'textarea',
      '#default_value' => $description_value,
      '#required' => TRUE,
  );

  $form['sbas'] = array (
      '#title' => t('Sociatal Benefit Areas'),
      '#type' => 'select',
      '#multiple' => TRUE,
      '#default_value' => $selected_sbas,
      '#options' => array_column(eneon_get_taxonomy_terms_like(ENEON_VOCAB_BUNDLE_SBA), 'name', 'tid'),
      '#description' => t('Sociatal Benefit Areas your issue may be referring to.'),
  );

  $form['themes'] = array (
      '#title' => t('Themes'),
      '#type' => 'select',
      '#multiple' => TRUE,
      '#default_value' => $selected_themes,
      '#options' => array_column(eneon_get_taxonomy_terms_like(ENEON_VOCAB_BUNDLE_THEME), 'name', 'tid'),
      '#description' => t('Themes your issue may be referring to.'),
  );

  return $form;
}

/**
 * Returns form elements for the 'location info' page of the wizard.
 *
 * This is the second step of the wizard. This step suggests networks
 * based on the metadata (e.g. SBAs, themes) set in the previous step.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard_network_suggestions($form, &$form_state) {
  $form = array();

  $current_step = &$form_state['step'];
  $values_previous_step = $form_state['step_information'][$current_step - 1]['stored_values'];

  $entity = $form_state[ENEON_BUNDLE_ISSUE];
  $wrapper = entity_metadata_wrapper('node', $entity);
  $wrapper->title->set(_eneon_get_form_value('title', $values_previous_step));
  $wrapper->{ENEON_ISSUE_DESCRIPTION}->set(_eneon_get_form_value('description', $values_previous_step));
  $wrapper->{ENEON_ISSUE_SELECTED_SBAS}->set(_eneon_get_form_value('sbas', $values_previous_step, array()));
  $wrapper->{ENEON_ISSUE_SELECTED_THEMES}->set(_eneon_get_form_value('themes', $values_previous_step, array()));

  $selected_themes = $wrapper->{ENEON_ISSUE_SELECTED_THEMES}->value();
  $selected_sbas = $wrapper->{ENEON_ISSUE_SELECTED_SBAS}->value();

  $form['title'] = array(
      '#title' => t('Title'),
      '#type' => 'item',
      '#markup' => $values_previous_step['title'],
      '#weight' => 0,
  );
  $form['description'] = array(
      '#title' => t('Issue description'),
      '#type' => 'item',
      '#resizable' => false,
      '#markup' => $values_previous_step['description'],
      '#weight' => 0,
  );

  $options = array(
      'classes' => array('horizontal-list', 'no-bullets'),
      'link_options' => array(
        'attributes' => array(
            'target' => '_blank',
        ),
      ),
  );
  $form['sbas'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="form-item form-type-item"><label>' . t('Selected SBAs') . '</label>',
      '#suffix' => '</div>',
      '#markup' => _eneon_render_taxonomy_linklist($selected_sbas, $options),
  );
  $form['themes'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="form-item form-type-item"><label>' . t('Selected Themes') . '</label>',
      '#suffix' => '</div>',
      '#markup' => _eneon_render_taxonomy_linklist($selected_themes, $options),
  );

  $form['network_suggestion_hint'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="suggested-networks-hint">' . t('The following networks could be useful for your issue. You can narrow down the list of networks here:') . '</div>',
  );

  $selected_networks = $wrapper->{ENEON_ISSUE_SELECTED_NETWORKS}->value();
  $suggested_networks = eneon_network_query(array(
      'sbas' => array_column($selected_sbas, 'tid'),
      'themes' => array_column($selected_themes, 'tid'),
  ));

  $selected_nids = array_column($selected_networks, 'nid');
  $suggested_nids = array_column($suggested_networks, 'nid');

  $suggested_selected_nids = array_intersect($selected_nids, $suggested_nids);
  $unsuggested_selected_nids = array_diff($selected_nids, $suggested_nids);

  $unsuggested_but_selected_networks = array_filter($selected_networks, function($network) use($suggested_nids) {
    return !in_array($network->nid, $suggested_nids);
  });
  // rewrite keys
  $unsuggested_but_selected_networks = array_combine($unsuggested_selected_nids, array_values($unsuggested_but_selected_networks));

  $form['fieldset_eneon_proposed_networks'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proposed Networks (#@count)', array('@count' => sizeof($suggested_networks))),
  );

  if (eneon_not_empty_array($suggested_networks)) {
    $checkboxes = _eneon_create_checkboxes_with_modal_links($suggested_networks);
    $form['fieldset_eneon_proposed_networks']['suggested_networks'] = array(
        '#type' => 'checkboxes',
        '#options' => $checkboxes,
        '#default_value' => $suggested_selected_nids,
        '#prefix' => '<div class="network-list">',
        '#suffix' => '</div>',
    );
  } else {
    $form['fieldset_eneon_proposed_networks']['no_networks'] = array(
        '#type' => 'item',
        '#markup' => t('No networks can be suggested based on current selections'),
    );
  }


  if (eneon_not_empty_array($unsuggested_but_selected_networks)) {
    $form['fieldset_eneon_unsuggested_but_selected_networks'] = array(
        '#type' => 'fieldset',
        '#title' => t('Networks not suggested anymore'),
    );

    $checkboxes = _eneon_create_checkboxes_with_modal_links($unsuggested_but_selected_networks);
    $form['fieldset_eneon_unsuggested_but_selected_networks']['unsuggested_but_selected_networks'] = array(
        '#type' => 'checkboxes',
        '#options' => $checkboxes,
        '#default_value' => array_values($unsuggested_selected_nids),
        '#prefix' => '<div class="network-list">',
        '#suffix' => '</div>',
    );
  }

  return $form;
}

function _eneon_create_checkboxes_with_modal_links($networks) {
  $suggested_networks = array_column($networks, 'title', 'nid');
  // set modal dialog styling
  ctools_include('modal');
  ctools_include('ajax');
  ctools_modal_add_js();
  $modal_style = array(
      'CToolsModal' => array(
          'modalSize' => array(
              'type' => 'fixed',
              'width' => 600,
              'height' => 300,
              // How much to remove from the inner content to make space for the
              // theming.
              'contentRight' => 28,
              'contentBottom' => 39,
          ),
          'closeText' => '',
      ),
  );
  $current_js = drupal_add_js($modal_style, 'setting');

  // create link opening modal dialog
  return array_combine(array_keys($networks), array_map(function($title, $id) {
       return ctools_modal_text_button($title, 'eneon/nojs/' . $id, $title);
    }, $suggested_networks, array_keys($suggested_networks)));
}

/**
 * Wizard form submit handler.
 *
 * This function:
 * - Saves away $form_state['values']
 * - Process all the form values.
 *
 * And now comes the magic of the wizard, the function that should handle all
 * the inputs from the user on each different step.
 *
 * This demonstration handler just do a drupal_set_message() with the
 * information collected on each different step of the wizard.
 *
 * @ingroup eneon
 */
function eneon_issue_wizard_submit($form, &$form_state) {
  global $user;
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
  $values_previous_step = $form_state['values'];

  $entity = $form_state[ENEON_BUNDLE_ISSUE];
  $wrapper = entity_metadata_wrapper('node', $entity);

  $get_selection_from = function($name) use ($values_previous_step) {
    $networks_selection_states = _eneon_get_form_value($name, $values_previous_step, array());
    return array_filter($networks_selection_states, function($item) { return is_string($item); });
  };
  $selected_networks = array_merge(
      $get_selection_from('suggested_networks'),
      $get_selection_from('unsuggested_but_selected_networks'));

  $wrapper->{ENEON_ISSUE_SELECTED_NETWORKS}->set(array_values(array_filter($selected_networks)));

  $wrapper->author->set($user);
  $wrapper->save();

  drupal_set_message(t('The issue: @title has been saved.', array('@title' => $wrapper->title->value())));

  // Redirect user to edited entity page.
  $node_uri = node_uri($entity);
  $form_state['redirect'] = $node_uri['path'];
}

function _eneon_get_form_value($name, $previous_values, $default=NULL) {
    return isset($previous_values[$name])
        ? $previous_values[$name]
        : $default;
}
