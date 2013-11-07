#include <stdio.h>
#include <stdlib.h>

struct treelist_node
{
	/* children list */
	struct treelist_node	* children, * last_child;
	/* next sibling in a list */
	struct treelist_node	* sibling;
	struct treelist_node	* next_visible, * prev_visible;
	int	nr_printed_children_lines;
	int	saved_nr_printed_children_lines;
	const char	* line_text;

	static int line_idx;
#define MAX_LINE_LEN		1024
	static char line_data[MAX_LINE_LEN];

	treelist_node(const char * str)
	{
		children = last_child = sibling = next_visible = prev_visible = 0;
		nr_printed_children_lines = saved_nr_printed_children_lines = 0;
		line_text = str;
	}
	void add_child(struct treelist_node * child)
	{
		if (!children)
			/* child list empty - initialize it */
			children = last_child = child;
		else
			last_child = last_child->sibling = child;
	}
	struct treelist_node * add_child(const char * str)
	{
		add_child(new struct treelist_node(str));
		return last_child;
	}
	void toggle_fold(void)
	{
		if (!children)
			/* nothing to do */
			return;
		if (next_visible == children)
		{
			(next_visible = sibling)->prev_visible = this;
		}
		else
		{
			next_visible->prev_visible;
		}
	}
	int compute_visibility_data(void)
	{
		int i;
		struct treelist_node * p;
		i = 0;
		if (!(next_visible = p = children))
			next_visible = sibling;
		if (next_visible)
			next_visible->prev_visible = this;

		while (p)
			i += p->compute_visibility_data(), p = p->sibling;
		return 1 + (nr_printed_children_lines = saved_nr_printed_children_lines = i);
	}
	void init_tree_printer(void)
	{
		line_idx = 0;
	}
	void print_tree(void)
	{
		dump_header();
		if (children)
			printf("[-] ");
		else
			printf("");
		printf(" %s\n", line_text);
		if (children)
		{
			struct treelist_node * p;
			if (!sibling && line_idx)
			{
				/* last children in the list - do some fixups */
				line_data[line_idx - 1] = ' ';
				line_data[line_idx - 2] = ' ';
				line_data[line_idx - 3] = ' ';
				line_data[line_idx - 4] = ' ';
			}
			line_data[line_idx++] = ' ';
			line_data[line_idx++] = '|';
			line_data[line_idx++] = ' ';
			line_data[line_idx++] = ' ';
			p = children;
			while (p)
			{
				p->print_tree(), p = p->sibling;
			}
			line_idx -= 4;
		}
	}
private:
	void dump_header(void)
	{
		int i, j;
		for (i = j = 0; i < line_idx; (j = (line_data[i] == '|') ? i : j), i++);
		for (i = 0; i < line_idx; printf("%c", (i > j) ? '-' : line_data[i]), i++);
	}
};

int treelist_node::line_idx = 0;
char treelist_node::line_data[MAX_LINE_LEN];

int main(void)
{
struct treelist_node * root = new struct treelist_node("root"), * p;

	root->init_tree_printer();
	root->add_child("child 1");
	root->add_child("child 2");
	p = root->add_child("child 3");
	p->add_child("yyy 0");
	p->add_child("yyy 1");
	p = p->add_child("yyy 2");
	p->add_child("yyy 0");
	p->add_child("yyy 1");
	root->add_child("child 4");
	root->add_child("child 5");
	p = root->add_child("child 6");
	p->add_child("zzz 0");
	p->add_child("zzz 1");
	p->add_child("zzz 2");
	root->print_tree();
	printf("hello, world\n");
	return 0;
}

