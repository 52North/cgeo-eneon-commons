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
 * - Apache License, version 2.0
 * - Apache Software License, version 1.0
 * - GNU Lesser General Public License, version 3
 * - Mozilla Public License, versions 1.0, 1.1 and 2.0
 * - Common Development and Distribution License (CDDL), version 1.0
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
require_once 'eneon.common.inc';
function eneon_network_ajax() {
  global $user;
  global $language;
  $output = "<div>Here I am " . date('c') . "</div>";

  $filter = $_REQUEST ['filter'];
  if (! isset($filter)) {
    return;
  }
  $selected_networks = $_REQUEST ['selectedNetworks'];
  $networks = array ();
  if (eneon_not_empty_array($selected_networks)) {
    $tmp_networks = eneon_network_load_multiple($selected_networks);
    if (eneon_not_empty_array($tmp_networks)) {
      $networks = $tmp_networks;
    }
  }

  $tmp_networks = eneon_network_query($filter);
  if (eneon_not_empty_array($tmp_networks)) {
    $networks = array_map("unserialize", array_unique(array_map("serialize", array_merge($networks, $tmp_networks))));
    // array is sorted on the bases of id
    sort($networks);
  }

  // render "teaser" of entities in a div or ul element
  if (sizeof($networks) > 0) {
    $output = array ();
    $i = 0;
    foreach ($networks as $network) {
      $prepared_network = entity_view(ENEON_ENTITY_NETWORK, array (
          $network
      ), 'teaser');
      $tmp = drupal_render($prepared_network);
      $checked = "";
      if (in_array($network->id, $selected_networks)) {
        $checked = "checked='checked'";
      }
      $output [] = "<li class='suggested-network'><label><input type='checkbox' name='selectedNetwork' value='" . $network->id . "' " . $checked . " /></label>" . $tmp . "</li>";
      $i ++;
    }
    $output = "<fieldset><ul id='suggested-networks'>" . implode($output) . "</ul></fieldset>";
  }
  print $output;
  exit();
}

