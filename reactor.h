/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reactor.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apereira <apereira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/24 15:54:20 by apereira          #+#    #+#             */
/*   Updated: 2025/09/24 15:55:29 by apereira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* ************************************************************************** */
/* TYPES                                                                      */
/* ************************************************************************** */

typedef void	(*callback)(void *obj, int new_value);
typedef int		callback_id;

enum { callback_invalid = 0 };

typedef int	(*compute1_t)(int);
typedef int	(*compute2_t)(int, int);

typedef enum e_cell_type
{
	input,
	compute1,
	compute2
}	cell_type_t;

struct cell;

typedef struct callback_node
{
	callback_id			id;
	void				*obj;
	callback			cb;
	struct callback_node	*next;
}	callback_node_t;

typedef struct cell_node
{
	struct cell		*cell;
	struct cell_node	*next;
}	cell_node_t;

typedef struct reactor
{
	struct cell		*cells;
}	reactor_t;

typedef struct cell
{
	reactor_t		*reactor;
	int				value;
	cell_type_t		type;
	callback_node_t	*callbacks;
	callback_id		next_callback_id;
	struct cell		*next;
	bool			changed_in_propagation;
	cell_node_t		*dependents;
}	cell_base_t;

typedef struct compute1_cell
{
	cell_base_t		base;
	struct cell		*dependency;
	compute1_t		compute;
}	compute1_cell_t;

typedef struct compute2_cell
{
	cell_base_t		base;
	struct cell		*dependency1;
	struct cell		*dependency2;
	compute2_t		compute;
}	compute2_cell_t;
