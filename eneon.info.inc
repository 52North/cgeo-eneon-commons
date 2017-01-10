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

// TODO more infos at
//  https://www.drupal.org/node/1021466
//  http://wolfgangziegler.net/introducing_entity_metadata

/**
 * Implements hook_entity_property_info_alter().
 */
function eneon_entity_property_info() {
  $info = array();

  $info[ENEON_BUNDLE_ISSUE] = array(
      'properties' => array(
          ENEON_ISSUE_DESCRIPTION => array(
              'label' => t('Issue description'),
              'description' => t('Description of the issue.'),
              'type' => 'text',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_ISSUE_SELECTED_SBAS => array(
              'label' => t('Issue selected SBAs'),
              'description' => t('Selected SBAs.'),
              'type' => 'list<taxonomy_term>',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_ISSUE_SELECTED_THEMES => array(
              'label' => t('Issue selected THEMES'),
              'description' => t('Selected Themes.'),
              'type' => 'list<taxonomy_term>',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_ISSUE_SELECTED_NETWORKS => array(
              'label' => t('Issue selected networks'),
              'description' => t('Selected networks related to this issue.'),
//               'type' => 'entityreference',
              'type' => 'list<' . ENEON_BUNDLE_NETWORK . '>',
              'setter callback' => 'entity_property_verbatim_set',
          ),
      ),
  );

  $info[ENEON_BUNDLE_NETWORK] = array(
      'properties' => array(
          ENEON_NETWORK_ID => array(
              'label' => t('Network ID'),
              'description' => t('The ID of the network.'),
              'type' => 'integer',
          ),
          ENEON_NETWORK_DESCRIPTION => array(
              'label' => t('Network description'),
              'description' => t('Description of the network.'),
              'type' => 'text_long',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_NETWORK_TYPE => array(
              'label' => t('Network type'),
              'description' => t('Type of the ENEON network'),
              'type' => 'text',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_NETWORK_URL => array(
              'label' => t('Network URL'),
              'description' => t('URL of the network.'),
              'type' => 'text',
              'setter callback' => 'entity_property_verbatim_set',
              'schema field' => 'url',
          ),
          ENEON_NETWORK_LOGO_URL => array(
              'label' => t('Network logo'),
              'description' => t('Logo of the network.'),
              'type' => 'text',
              'setter callback' => 'entity_property_verbatim_set',
              'schema field' => 'logo_url',
          ),
          ENEON_NETWORK_SBA_REFERENCE => array(
              'label' => t('Network SBA references'),
              'description' => t('SBAs related to this network.'),
              'type' => 'list<taxonomy_term_reference>',
              'setter callback' => 'entity_property_verbatim_set',
          ),
          ENEON_NETWORK_THEME_REFERENCE => array(
              'label' => t('Network theme reference'),
              'description' => t('Themes related to this network.'),
              'type' => 'list<taxonomy_term_reference>',
              'setter callback' => 'entity_property_verbatim_set',
          ),
      ),
  );

  return $info;
}
