#include <stdlib.h>
#include <stdio.h>
#include "xdebug_mm.h"
#include "xdebug_str.h"
#include "xdebug_xml.h"

static void xdebug_xml_return_attribute(xdebug_xml_attribute* attr, xdebug_str* output)
{
	xdebug_str_addl(output, " ", 1, 0);
	xdebug_str_add(output, attr->name, 0);
	xdebug_str_addl(output, "=\"", 2, 0);
	xdebug_str_add(output, attr->value, 0);
	xdebug_str_addl(output, "\"", 1, 0);
	
	if (attr->next) {
		xdebug_xml_return_attribute(attr->next, output);
	}
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output)
{
	xdebug_str_addl(output, "<", 1, 0);
	xdebug_str_add(output, node->tag, 0);

	if (node->attribute) {
		xdebug_xml_return_attribute(node->attribute, output);
	}
	xdebug_str_addl(output, ">", 1, 0);

	if (node->child) {
		xdebug_xml_return_node(node->child, output);
	}

	if (node->text) {
		if (strstr(node->text, "]]>")) {
			/* if cdata tags are in the text, then we must base64 encode */
			int new_len = 0;
			char *encoded_text = xdebug_base64_encode(node->text, strlen(node->text), &new_len);
			xdebug_str_add(output, encoded_text, 0);
		} else {
			xdebug_str_addl(output, "<![CDATA[", 9, 0);
			xdebug_str_add(output, node->text, 0);
			xdebug_str_addl(output, "]]>", 3, 0);
		}
	}

	xdebug_str_addl(output, "</", 2, 0);
	xdebug_str_add(output, node->tag, 0);
	xdebug_str_addl(output, ">", 1, 0);

	if (node->next) {
		xdebug_xml_return_node(node->next, output);
	}
}

xdebug_xml_node *xdebug_xml_node_init_ex(char *tag, int free_tag)
{
	xdebug_xml_node *xml = xdmalloc(sizeof (xdebug_xml_node));

	xml->tag = tag;
	xml->text = NULL;
	xml->child = NULL;
	xml->attribute = NULL;
	xml->next = NULL;
	xml->free_tag = free_tag;

	return xml;
}

void xdebug_xml_add_attribute_ex(xdebug_xml_node* xml, char *attribute, char *value, int free_name, int free_value)
{
	xdebug_xml_attribute *attr = xdmalloc(sizeof (xdebug_xml_attribute));
	xdebug_xml_attribute **ptr;

	/* Init structure */
	attr->name = attribute;
	attr->value = value;
	attr->next = NULL;
	attr->free_name = free_name;
	attr->free_value = free_value;

	/* Find last attribute in node */
	ptr = &xml->attribute;
	while (*ptr != NULL) {
		ptr = &(*ptr)->next;
	}
	*ptr = attr;
}

void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child)
{
	xdebug_xml_node **ptr;

	ptr = &xml->child;
	while (*ptr != NULL) {
		ptr = &((*ptr)->next);
	}
	*ptr = child;
}

void xdebug_xml_add_text(xdebug_xml_node *xml, char *text)
{
	if (xml->text) {
		xdfree(xml->text);
	}
	xml->text = text;
	if (strstr(xml->text, "]]>")) {
		/* if cdata tags are in the text, then we must base64 encode */
		xdebug_xml_add_attribute_ex(xml, "encoding", "base64", 0, 0);
	}
}

static void xdebug_xml_attribute_dtor(xdebug_xml_attribute *attr)
{
	if (attr->next) {
		xdebug_xml_attribute_dtor(attr->next);
	}
	if (attr->free_name) {
		xdfree(attr->name);
	}
	if (attr->free_value) {
		xdfree(attr->value);
	}
	xdfree(attr);
}

void xdebug_xml_node_dtor(xdebug_xml_node* xml)
{
	if (xml->next) {
		xdebug_xml_node_dtor(xml->next);
	}
	if (xml->child) {
		xdebug_xml_node_dtor(xml->child);
	}
	if (xml->attribute) {
		xdebug_xml_attribute_dtor(xml->attribute);
	}
	if (xml->free_tag) {
		xdfree(xml->tag);
	}
	if (xml->text) {
		xdfree(xml->text);
	}
	xdfree(xml);
}