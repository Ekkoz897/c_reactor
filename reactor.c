/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reactor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apereira <apereira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/24 16:01:51 by apereira          #+#    #+#             */
/*   Updated: 2025/09/25 19:26:58 by apereira         ###   ########.fr       */
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

/*
** Run all registered callbacks for a given cell.
*/
static void	fire_cell_callbacks(struct cell *cell)
{
	callback_node_t	*cbn;

	for (cbn = cell->callbacks; cbn; cbn = cbn->next)
		cbn->cb(cbn->obj, cell->value);
}

static void	init_input_cell(struct cell *cell, int initial_value)
{
	cell->value = initial_value;
}

/*
** Initialize a compute1 cell depending on one input.
*/
static void	init_compute1_cell(
	struct cell *cell, struct cell *dep, compute1_t compute)
{
	compute1_cell_t	*c1;

	c1 = (compute1_cell_t *)cell;
	c1->dependency = dep;
	c1->compute = compute;
	c1->base.value = compute(dep->value);
}

/*
** Initialize a compute2 cell depending on two inputs.
*/
static void	init_compute2_cell(
	struct cell *cell, struct cell *dep1,
	struct cell *dep2, compute2_t compute)
{
	compute2_cell_t	*c2;

	c2 = (compute2_cell_t *)cell;
	c2->dependency1 = dep1;
	c2->dependency2 = dep2;
	c2->compute = compute;
	c2->base.value = compute(dep1->value, dep2->value);
}

static void	add_cell_to_reactor(reactor_t *r, struct cell *cell)
{
	cell->next = r->cells;
	r->cells = cell;
}

static void	add_dependent(struct cell *dependency, struct cell *dependent)
{
	cell_node_t	*node;

	node = critical_malloc(sizeof(*node), "add_dependent");
	node->cell = dependent;
	node->next = dependency->dependents;
	dependency->dependents = node;
}

/* ************************************************************************** */
/* COMPUTATION & PROPAGATION                                                  */
/* ************************************************************************** */

static int	compute_cell_value(struct cell *cell)
{
	if (cell->type == compute1)
	{
		compute1_cell_t	*c1;

		c1 = (compute1_cell_t *)cell;
		return (c1->compute(c1->dependency->value));
	}
	if (cell->type == compute2)
	{
		compute2_cell_t	*c2;

		c2 = (compute2_cell_t *)cell;
		return (c2->compute(
				c2->dependency1->value, c2->dependency2->value));
	}
	return (cell->value);
}

static bool	update_single_pass(reactor_t *r)
{
	struct cell	*cell;
	int			new_value;
	bool		any_changed;

	any_changed = false;
	for (cell = r->cells; cell; cell = cell->next)
	{
		if (cell->type == input)
			continue ;
		new_value = compute_cell_value(cell);
		if (new_value == cell->value)
			continue ;
		cell->value = new_value;
		cell->changed_in_propagation = true;
		any_changed = true;
	}
	return (any_changed);
}

static void	update_values_until_stable(reactor_t *r)
{
	while (update_single_pass(r))
		;
}

static void	fire_callbacks_for_changed_cells(reactor_t *r)
{
	struct cell	*cell;

	for (cell = r->cells; cell; cell = cell->next)
	{
		if (cell->changed_in_propagation)
		{
			fire_cell_callbacks(cell);
			cell->changed_in_propagation = false;
		}
	}
}

/*
** After a change in an input cell, update values and fire callbacks.
*/
static void	propagate_changes(struct cell *start_cell)
{
	reactor_t	*r;

	r = start_cell->reactor;
	update_values_until_stable(r);
	fire_callbacks_for_changed_cells(r);
}
