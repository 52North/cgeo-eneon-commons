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

function eneon_view_network_list() {
  $output = array();
  $entities = entity_load(ENEON_ENTITY_NETWORK);
  // when listing entities, we use the teaser view
  $list = entity_view(ENEON_ENTITY_NETWORK, $entities, 'teaser');
  if (isset($list[ENEON_ENTITY_NETWORK])) {
    foreach ($list[ENEON_ENTITY_NETWORK] as $entity) {
        $output[] = drupal_render($entity);
    }
  }
  return implode($output);
}
function eneon_view_issue_list() {
  // Show link to the entity "Add" page.
  $output = '<br/>';
  $output .= theme_link(
      array(
          'text' => t('Add new entity'),
          'path' => ENEON_PATH_ISSUES . '/add',
          'options' => array(
              'attributes' => array(),
              'html' => TRUE,
          ),
      )
      );

  // Show list of existing entities.
  $entities = entity_load(ENEON_ENTITY_ISSUE);
  $items = array();
  foreach ($entities as $entity) {
    $entity_uri = entity_uri(ENEON_ENTITY_ISSUE, $entity);
    $items[] = l(entity_label(ENEON_ENTITY_ISSUE, $entity), $entity_uri['path']);
  }
  $output .= theme_item_list(
      array(
          'items' => $items,
          'title' => t('List of existing entities.'),
          'type' => 'ul',
          'attributes' => array(),
      )
  );

  return $output;
}
// function eneon_view_entity_item($id, $entity_type) {
function eneon_view_entity_item($entity) {
  drupal_set_title($entity->title);
  return entity_view(ENEON_ENTITY_ISSUE, array(
      entity_id(ENEON_ENTITY_ISSUE, $entity) => $entity),
      'full');
}
/**
 * Creates an entity stub and delegates to the edit form.
 *
 * @param type $entity_type the entity type
 * @return The entity's edit form
 */
// function eneon_add_entity_item($entity_type) {
//   drupal_set_title(t('Add new ' . entity_get_info($entity_type)['label']));
//   $entity = entity_create($entity_type, array());
//   return entity_ui_get_form($entity_type, $entity);
// }
