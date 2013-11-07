#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <string.h>


#define random rand

bool harr[1024 * 128];

static void gen_graph(bool * graph_data, int nr_nodes, int nr_edges)
{
int i;	
	memset(graph_data, false, nr_nodes * nr_nodes * sizeof * graph_data);

	for (i = 0; i < nr_edges; i ++)
		graph_data[random() % (nr_nodes * nr_nodes)] = true;
}


static void gen_perm_row(unsigned char * perm, int nr_args)
{
int i, t;
unsigned char c;

	for (i = 0; i < nr_args; i ++)
		perm[i] = i;
	for (i = nr_args; i > 1; i--)
	{
		t = random() % i;
		c = perm[i - 1];
		perm[i - 1] = perm[t];
		perm[t] = c;
	}
}

static void gen_perm(unsigned char * perm, int nr_args, int nr_nodes)
{
int i, j, t;
unsigned char c;
unsigned char * p;

	for (j = 0; j < nr_nodes; j ++)
	{
		p = perm + j * nr_args;
		gen_perm_row(p, nr_args);
	}
}

static void gen_code(unsigned char * perm, int nr_args, int node_idx)
{
int i;	
	printf("static inline int foo%03i(", node_idx);
	for (i = 0; i < nr_args; i++)
		printf("int arg%02i, ", i);
	printf(")\n{\t int ");
	for (i = 0; i < nr_args; i++)
		printf("t%02i, ", i);
	printf("dummy;\n\n");
	for (i = 0; i < nr_args; i ++)
		printf("\tt%02i = arg%02i;\n", (perm + node_idx * nr_args)[i], i);
	printf("\n\tdummy = ");
	for (i = 0; i < nr_args; i ++)
		printf("t%02i + ", (perm + node_idx * nr_args)[i]);
}

static void dfs(unsigned char * perm, bool * graph, bool * is_visited, int nr_args, int node_count, int node_idx)
{
int i, j;

	if (is_visited[node_idx])
	{
		printf("error\n");
		exit(1);
	}
	is_visited[node_idx] = true;
	gen_code(perm, nr_args, node_idx);
	for (i = 0; i < node_count; i ++)
	{
		if ((graph + node_count * node_idx)[i] && !is_visited[i])
		{
			printf("foo%03i(", i);
			for (j = 0; j < nr_args; j++)
			{
				printf("t%02i", (perm + node_idx * nr_args)[j]);
				if (j != nr_args - 1)
					printf(", ");
			}
			printf(") + ");
		}
	}
	printf("1;\n\n\treturn dummy;\n}\n\n");
	for (i = 0; i < node_count; i ++)
		if ((graph + node_count * node_idx)[i] && !is_visited[i])
			dfs(perm, graph, is_visited, nr_args, node_count, i);
}


#define NR_NODES	10
#define NR_ARGS		3
#define NR_EDGES	20

static bool g[NR_NODES][NR_NODES];
static bool v[NR_NODES];
static unsigned char p[NR_NODES][NR_ARGS];

int main(void)
{
int i;	
	gen_graph(g, NR_NODES, NR_EDGES);
	gen_perm(p, NR_ARGS, NR_NODES);
	memset(v, false, sizeof v);

	for (i = 0; i < NR_NODES; i ++)
		if (!v[i])
			dfs(p, g, v, NR_ARGS, NR_NODES, i);

	return 0;
} 
