#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

void parseStory (xmlDocPtr doc, xmlNodePtr cur)
{

    xmlChar* key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar*)"Value")))
        {
            key = xmlNodeGetContent(cur);
            printf("Revs x100: %s\n", (char*) key);
            xmlFree(key);
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar*)"TimerValue")))
        {
            key = xmlNodeGetContent(cur);
            printf("Value to send to RevBurner: %s\n", (char*) key);
            xmlFree(key);
        }
        cur = cur->next;
    }
    return;
}

static void parseDoc(char* docname)
{

    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(docname);

    if (doc == NULL )
    {
        fprintf(stderr,"Document not parsed successfully. \n");
        return;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(doc);
        return;
    }

    if (xmlStrcmp(cur->name, (const xmlChar*) "TachometerSettings"))
    {
        fprintf(stderr,"document of the wrong type, root node != TachometerSettings");
        xmlFreeDoc(doc);
        return;
    }

    cur = cur->xmlChildrenNode;
    cur = cur->next;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {

        if (!(xmlStrcmp(cur->name, (const xmlChar*)"SettingsItem")))
        {
            parseStory (doc, cur);
        }

        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return;
}

int main(int argc, char** argv)
{

    char* docname;

    if (argc <= 1)
    {
        printf("Usage: %s docname\n", argv[0]);
        return(0);
    }

    docname = argv[1];
    parseDoc (docname);

    return (1);
}
