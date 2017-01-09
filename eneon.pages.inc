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

function eneon_view_network_list() {
  $output = array();
  $entities = eneon_network_query();
  foreach ($entities as $node) {
    $output[] = node_view($node, 'teaser');
  }
  return drupal_render($output);
}

function eneon_view_issue_list() {
  // Show link to the entity "Add" page.
  $output = '<br/>';
  $output .= theme_link(
      array(
          'text' => t('Add new Issue'),
          'path' => 'issues/add',
          'options' => array(
              'attributes' => array(),
              'html' => TRUE,
          ),
      )
  );

  $entities = eneon_issue_query();
  foreach ($entities as $node) {
    $node_view = node_view($node, 'teaser');
    $output .= drupal_render($node_view);
  }
  return $output;
}

function eneon_view_network_modal($js = NULL, $nid= null) {
  $node = node_load($nid);
  $output = node_view($node, 'full');
  $contents = render($output);
  if ($js) {
    // Required includes for ctools to work:
    ctools_include('modal');
    ctools_include('ajax');
    return ctools_modal_render($node->title, $contents) ;
  } else {
    return $contents;
  }
}
