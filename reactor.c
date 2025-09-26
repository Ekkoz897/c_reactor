/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reactor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apereira <apereira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/24 16:01:51 by apereira          #+#    #+#             */
/*   Updated: 2025/09/26 13:02:00 by apereira         ###   ########.fr       */
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

/* ************************************************************************** */
/* HELPERS                                                                    */
/* ************************************************************************** */

/*
** critical_malloc:
** Safe malloc: aborts on failure.
*/
static void	*critical_malloc(size_t size, const char *context)
{
	void	*ptr;

	ptr = malloc(size);
	if (ptr)
		return (ptr);
	fprintf(stderr, "fatal: allocation failed in %s\n", context);
	abort();
	return (NULL);
}

static void	free_callbacks(callback_node_t *head)
{
	callback_node_t	*next;

	while (head)
	{
		next = head->next;
		free(head);
		head = next;
	}
}

static void	free_dependents(cell_node_t *head)
{
	cell_node_t	*next;

	while (head)
	{
		next = head->next;
		free(head);
		head = next;
	}
}

static callback_node_t	*find_callback_node(struct cell *cell, callback_id id)
{
	callback_node_t	*node;

	for (node = cell->callbacks; node; node = node->next)
		if (node->id == id)
			return (node);
	return (NULL);
}

static void	unlink_callback_node(struct cell *cell, callback_node_t *node)
{
	callback_node_t	**indirect;

	indirect = &cell->callbacks;
	while (*indirect && *indirect != node)
		indirect = &(*indirect)->next;
	if (*indirect == node)
	{
		*indirect = node->next;
		free(node);
	}
}


/* ************************************************************************** */
/* PUBLIC API                                                                 */
/* ************************************************************************** */

void	destroy_reactor(reactor_t *r)
{
	struct cell	*current;
	struct cell	*next;

	if (!r)
		return ;
	current = r->cells;
	while (current)
	{
		next = current->next;
		free_callbacks(current->callbacks);
		free_dependents(current->dependents);
		free(current);
		current = next;
	}
	free(r);
}

struct cell	*create_input_cell(reactor_t *r, int initial_value)
{
	struct cell	*cell;

	cell = allocate_input_cell(r);
	if (!cell)
		return (NULL);
	init_input_cell(cell, initial_value);
	add_cell_to_reactor(r, cell);
	return (cell);
}

struct cell	*create_compute1_cell(
	reactor_t *r, struct cell *dep, compute1_t compute)
{
	struct cell	*cell;

	cell = allocate_compute1_cell(r);
	if (!cell)
		return (NULL);
	init_compute1_cell(cell, dep, compute);
	add_dependent(dep, cell);
	add_cell_to_reactor(r, cell);
	return (cell);
}

struct cell	*create_compute2_cell(
	reactor_t *r, struct cell *dep1,
	struct cell *dep2, compute2_t compute)
{
	struct cell	*cell;

	cell = allocate_compute2_cell(r);
	if (!cell)
		return (NULL);
	init_compute2_cell(cell, dep1, dep2, compute);
	add_dependent(dep1, cell);
	add_dependent(dep2, cell);
	add_cell_to_reactor(r, cell);
	return (cell);
}

int	get_cell_value(struct cell *cell)
{
	return (cell->value);
}

void	set_cell_value(struct cell *cell, int new_value)
{
	if (!cell || cell->type != input)
		return ;
	if (cell->value == new_value)
		return ;
	cell->value = new_value;
	propagate_changes(cell);
}

callback_id	add_callback(struct cell *cell, void *obj, callback cb)
{
	callback_node_t	*node;

	if (!cell || !cb)
		return (callback_invalid);
	node = malloc(sizeof(*node));
	if (!node)
		return (callback_invalid);
	node->id = cell->next_callback_id++;
	node->obj = obj;
	node->cb = cb;
	node->next = cell->callbacks;
	cell->callbacks = node;
	return (node->id);
}

void	remove_callback(struct cell *cell, callback_id id)
{
	callback_node_t	*node;

	if (!cell || id == callback_invalid)
		return ;
	node = find_callback_node(cell, id);
	if (node)
		unlink_callback_node(cell, node);
}
