/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reactor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apereira <apereira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/24 16:01:51 by apereira          #+#    #+#             */
/*   Updated: 2025/09/24 16:02:01 by apereira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "reactor.h"


/* ************************************************************************** */
/* CORE OPERATIONS                                                            */
/* ************************************************************************** */

static struct cell	*allocate_input_cell(reactor_t *r)
{
	struct cell	*cell;

	cell = calloc(1, sizeof(cell_base_t));
	if (!cell)
		return (NULL);
	cell->reactor = r;
	cell->type = input;
	cell->next_callback_id = 1;
	return (cell);
}

static struct cell	*allocate_compute1_cell(reactor_t *r)
{
	struct cell	*cell;

	cell = calloc(1, sizeof(compute1_cell_t));
	if (!cell)
		return (NULL);
	cell->reactor = r;
	cell->type = compute1;
	cell->next_callback_id = 1;
	return (cell);
}

static struct cell	*allocate_compute2_cell(reactor_t *r)
{
	struct cell	*cell;

	cell = calloc(1, sizeof(compute2_cell_t));
	if (!cell)
		return (NULL);
	cell->reactor = r;
	cell->type = compute2;
	cell->next_callback_id = 1;
	return (cell);
}
