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


require_once 'includes/eneon.common.inc';

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

  $create_new = !isset($entity) && !isset($form_state[ENEON_ENTITY_ISSUE]);
  $entity = isset($form_state[ENEON_ENTITY_ISSUE]) ? $form_state[ENEON_ENTITY_ISSUE] : $entity;
  $form_state[ENEON_ENTITY_ISSUE] = $create_new
      ? entity_create(ENEON_ENTITY_ISSUE, array('is_new_revision' => TRUE))
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

  _eneon_write_entity_values($form_state);
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

  $entity = $form_state[ENEON_ENTITY_ISSUE];
  $wrapper = $entity->wrapper();

  $title_value = !empty($form_state['values']['title']) ? $form_state['values']['title'] : $wrapper->title->value();
  $description_value = !empty($form_state['values']['description']) ? $form_state['values']['description'] : $wrapper->description->value();
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

  $entity = $form_state[ENEON_ENTITY_ISSUE];
  $wrapper = entity_metadata_wrapper(ENEON_ENTITY_ISSUE, $entity);
  $selected_themes = !empty($values_previous_step['themes']) ? $values_previous_step['themes'] : $wrapper->{ENEON_ISSUE_SELECTED_THEMES}->value();
  $selected_sbas = !empty($values_previous_step['sbas']) ? $values_previous_step['sbas'] : $wrapper->{ENEON_ISSUE_SELECTED_SBAS}->value();

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

  // XXX values are only rendered after saving (ie when editing issue)
  $form['sbas'] = field_view_field(ENEON_ENTITY_ISSUE, $entity, ENEON_ISSUE_SELECTED_SBAS);
  $form['themes'] = field_view_field(ENEON_ENTITY_ISSUE, $entity, ENEON_ISSUE_SELECTED_THEMES);

  $form['network_suggestion_hint'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="suggested-networks-hint">' . t('The following networks could be useful for your issue. You can narrow down the list of networks here:') . '</div>',
  );

  $networks = eneon_network_query(array(
      'sbas' => $selected_sbas,
      'themes' => $selected_themes,
  ));
  sort($networks);

  $form['fieldset_eneon_proposed_networks'] = array(
      '#type' => 'fieldset',
      '#title' => t('Proposed Networks'),
  );

  $selected_networks = array();
  // TODO always show selected networks (even they do not match actual query)

  if (eneon_not_empty_array($networks)) {
    $suggested_networks = array();
    foreach ($networks as $network) {
      $suggested_networks[$network->id] = $network->title;
    }
    $form['fieldset_eneon_proposed_networks']['suggested_networks'] = array(
        '#type' => 'checkboxes',
        '#options' => $suggested_networks,
        '#prefix' => '<div class="suggested-networks">',
        '#suffix' => '</div>',
    );
  }
  return $form;
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
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];

  _eneon_write_entity_values($form_state);
  $entity = $form_state[ENEON_ENTITY_ISSUE];
  $entity->save();

  drupal_set_message(t('The issue: @title has been saved.', array('@title' => entity_label(ENEON_ENTITY_ISSUE, $entity))));

  // Redirect user to edited entity page.
  $entity_uri = entity_uri(ENEON_ENTITY_ISSUE, $entity);
  $form_state['redirect'] = $entity_uri['path'];
}

function _eneon_write_entity_values(&$form_state) {
  $form_value = function($name, $step_nr, $default=NULL) use (&$form_state) {
    $step_values = $form_state['step_information'][$step_nr];
    return isset($step_values['stored_values'][$name])
    ? $step_values['stored_values'][$name]
    : $default;
  };

  $entity = &$form_state[ENEON_ENTITY_ISSUE];
  $wrapper = $entity->wrapper();

  $wrapper->title->set($form_value('title', 1));
  $wrapper->description->set($form_value('description', 1));
  $wrapper->{ENEON_ISSUE_SELECTED_SBAS}->set($form_value('sbas', 1, array()));
  $wrapper->{ENEON_ISSUE_SELECTED_THEMES}->set($form_value('themes', 1, array()));

  $network_checkbox_states = $form_value('suggested_networks', 2, array());
  $selected_networks = array_filter($network_checkbox_states, function($item) { return $item != 0; });
  $wrapper->{ENEON_ISSUE_SELECTED_NETWORKS}->set($selected_networks);
}