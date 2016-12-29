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

  $sba_vocab = taxonomy_vocabulary_machine_name_load(ENEON_VOCAB_BUNDLE_SBA);

  $networks = $eneon_graph['network'];
  foreach ($networks as $network) {
    if (! isset($network ['id']) || ! isset($network ['title'])) {
      continue;
    }

    $sbas = isset($network ['SBA']) ? $network ['SBA'] : array();
    $known_sba_names = array_map(function($term) { return $term->name; }, eneon_get_sbas_like());

    foreach ($sbas as $sba) {
      if ( $sba !== 'All' && !in_array($sba, $known_sba_names)) {
        $term = (object) array(
            'name' => $sba,
            'description' => $sba,
            'vid' => $sba_vocab->vid,
        );
        taxonomy_term_save($term);
      }
    }

    $entity = eneon_create_or_load_network($network);

    if ($entity) {
      // Add additional attributes
      $entity->description = $network ['description'];
      $entity->type = $network ['type'];

      // Add additional attributes
      $wrapper = entity_metadata_wrapper(ENEON_ENTITY_NETWORK, $entity);
      $existing_relations = $wrapper->{ENEON_NETWORK_SBA_REFERENCE}->value();

      $related_sbas = eneon_get_sbas_like($sbas);
      foreach($related_sbas as $related_sba) {
        if ( !in_array($related_sba, $existing_relations)) {
          $existing_relations[] = $related_sba;
        }
      }
      $wrapper->{ENEON_NETWORK_SBA_REFERENCE}->set($existing_relations);
      $wrapper->save();
    }
  }
  drupal_set_message($message = t('"eneon" module: imported #@count networks.', array('@count' => sizeof($networks))), $type = 'status');
}

function eneon_get_sbas_like($filters=array('*')) {
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
      ->entityCondition('bundle', ENEON_VOCAB_BUNDLE_SBA);

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
        ->condition('r.' . ENEON_NETWORK_SBA_REFERENCE . '_target_id', $sbas, 'IN')
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
