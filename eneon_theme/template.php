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

// /**
//  * Implements hook_theme()
//  */
// function eneon_theme_preprocess_page(&$variables, $hook) {
//   global $user;

//   // http://www.zyxware.com/articles/2044/drupal-tips-how-to-add-dynamic-login-logout-link-to-your-primary-menu
//   if ($user->uid != 0) {
//     $new_links['account-link'] = array(
//         'attributes' => array('title' => 'Account link'),
//         'href' => 'user',
//         'title' => t('My Account'),
//     );
//     $new_links['logout-link'] = array(
//         'attributes' => array('title' => 'Logout link'),
//         'href' => 'user/logout',
//         'title' => t('Log out'),
//     );
//   } else {
//     $new_links['login-link'] = array(
//         'attributes' => array('title' => 'Login link'),
//         'href' => 'user/login',
//         'title' => t('Login'),
//     );
//   }
//   $merge_with = isset($variables['secondary_links']) ? $variables['secondary_links'] : array();
//   $variables['secondary_links'] = array_merge_recursive($merge_with, $new_links);
// }