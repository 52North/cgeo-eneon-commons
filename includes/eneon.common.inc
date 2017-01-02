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
function eneon_download_network_graph() {
  $response = drupal_http_request('http://www.eneon.net/graph/networks.json');
  $data = ! empty($response) && $response->code == 200 ? $response->data : NULL;
  if (empty($data)) {
    drupal_set_message($message = t('"eneon" module: no network graph data downloaded.'), $type = 'warning');
    return FALSE;
  }
  $networks = drupal_json_decode($data);
  $file_object = file_save_data($data, "public://eneon_networks.json", FILE_EXISTS_REPLACE);
  if ( !$file_object) {
    drupal_set_message($message = t('"eneon" module: could save/update file.'), $type = 'warning');
    return FALSE;
  }
  file_save($file_object);
  $file_object->filemime = 'application/json';
  return $networks;
}

function eneon_update_networks() {
  $eneon_graph = eneon_download_network_graph();
  if ( !$eneon_graph) {
    drupal_set_message($message = t('"eneon" module: could save/update networks.'), $type = 'warning');
    return;
  }

  $networks = $eneon_graph['network'];

  /*
   * Some networks refer to 'All' SBAs so we have to iterate through
   * all SBAs referenced in the network
   *
   * TODO probably its better to define them directly as constants?!
   */

  // first iteration creates all SBAs/themes/etc.
  eneon_create_taxonomy_terms($networks, 'SBA', ENEON_VOCAB_BUNDLE_SBA);
  eneon_create_taxonomy_terms($networks, 'theme', ENEON_VOCAB_BUNDLE_THEME);

  // second iteration creates all networks
  foreach ($networks as $network) {
    if (! isset($network ['id']) || ! isset($network ['title'])) {
      continue;
    }

    $entity = eneon_create_or_load_network($network);

    if ($entity) {
      // Add additional attributes
      $entity->description = $network ['description'];
      $entity->type = $network ['type'];

      // Add additional attributes
      $wrapper = entity_metadata_wrapper(ENEON_ENTITY_NETWORK, $entity);
      eneon_add_theme_references($wrapper, $network);
      eneon_add_sba_references($wrapper, $network);
      $wrapper->save();
    }
  }
  drupal_set_message($message = t('"eneon" module: imported #@count networks.', array('@count' => sizeof($networks))), $type = 'status');
}

function eneon_create_taxonomy_terms($networks, $property_name, $vocab_name) {
  $vocab = taxonomy_vocabulary_machine_name_load($vocab_name);
  foreach ($networks as $network) {
    if (! isset($network ['id']) || ! isset($network ['title'])) {
      continue;
    }

    $terms = isset($network [$property_name]) ? $network [$property_name] : array();
    $known_sba_names = array_map(function($term) { return $term->name; }, eneon_get_taxonomy_terms_like($vocab_name));

    foreach ($terms as $term_name) {
      if ( $term_name !== 'All' && !in_array($term_name, $known_sba_names)) {
        $term = (object) array(
            'name' => $term_name,
            'description' => $term_name,
            'vid' => $vocab->vid,
        );
        taxonomy_term_save($term);
      }
    }
  }
}

function eneon_add_theme_references($wrapper, $network) {
  $theme_relations = $wrapper->{ENEON_NETWORK_THEME_REFERENCE}->value();

  $themes = isset($network ['theme']) ? $network ['theme'] : array();
  $themes = in_array('All', $themes) ? array('*') : $themes;
  $theme_terms = eneon_get_taxonomy_terms_like(ENEON_VOCAB_BUNDLE_THEME, $themes);
  foreach($theme_terms as $theme_term) {
    if ( !in_array($theme_term, $theme_relations)) {
      $theme_relations[] = $theme_term;
    }
  }
  $wrapper->{ENEON_NETWORK_THEME_REFERENCE}->set($theme_relations);
}

function eneon_add_sba_references($wrapper, $network) {
  $sba_relations = $wrapper->{ENEON_NETWORK_SBA_REFERENCE}->value();

  $sbas = isset($network ['SBA']) ? $network ['SBA'] : array();
  $sbas = in_array('All', $sbas) ? array('*') : $sbas;
  $sba_terms = eneon_get_taxonomy_terms_like(ENEON_VOCAB_BUNDLE_SBA, $sbas);
  foreach($sba_terms as $sba_term) {
    if ( !in_array($sba_term, $sba_relations)) {
      $sba_relations[] = $sba_term;
    }
  }
  $wrapper->{ENEON_NETWORK_SBA_REFERENCE}->set($sba_relations);
}

function eneon_get_taxonomy_terms_like($vocab, $filters=array('*')) {
  if (!isset($filters) || empty($filters)) {
    return array();
  }
  // https://www.drupal.org/docs/7/creating-custom-modules/howtos/how-to-use-entityfieldquery-for-drupal-7
  $query = new EntityFieldQuery();
  $or_values = array();
  if ( !in_array('*', $filters)) {
    foreach ($filters as $filter) {
      $or_values[] = $filter;
    }
  }

  if ( !empty($or_values)) {
    $query = $query->propertyCondition('name', $or_values, 'IN');
  }

  $query = $query
      ->entityCondition('entity_type', 'taxonomy_term')
      ->entityCondition('bundle', $vocab);

  $results = $query->execute();
  if (isset($results['taxonomy_term'])) {
      $term_ids = array_keys($results['taxonomy_term']);
      return entity_load('taxonomy_term', $term_ids);
  } else {
    return array();
  }
}

function eneon_create_or_load_network($network) {
  $nid = $network ['id'];
  $title = $network ['title'];
  $existing_networks = db_select(ENEON_TABLE_NETWORKS, 'n')
      ->fields('n', array ('id'))
      ->fields('n', array('nid'))
      ->execute()
      // returns an indexed array
      ->fetchAllAssoc('id');

  $nidsById = array_column($existing_networks, 'nid', 'id');
  $idsByNid = array_flip($nidsById);
  return in_array($nid, $nidsById)
      ? entity_load_single(ENEON_ENTITY_NETWORK, $idsByNid[$nid])
      : entity_create(ENEON_ENTITY_NETWORK, array (
          'nid' => $nid,
          'title' => $title,
        ));
}

function eneon_network_query($filter=array()) {
  $sbas = isset($filter['sbas']) && is_array($filter['sbas']) ? $filter['sbas'] : array();
  $themes = isset($filter['themes']) && is_array($filter['themes']) ? $filter['themes'] : array();
  $nids = array();

  if (eneon_not_empty_array($sbas)) {
    $nids = db_select('field_data_' . ENEON_NETWORK_SBA_REFERENCE, 'r')
        ->fields('r', array('entity_id'))
        ->condition('r.' . ENEON_NETWORK_SBA_REFERENCE . '_tid', $sbas, 'IN')
        ->execute()
        ->fetchCol();
  }

  return eneon_not_empty_array($nids)
      ? eneon_network_load_multiple($nids)
      : array();
}

function eneon_not_empty_array($potential_array) {
  return isset($potential_array) &&
  is_array($potential_array) &&
  sizeof($potential_array) > 0;
}
