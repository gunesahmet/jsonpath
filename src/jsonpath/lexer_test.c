#include "lexer.h"
#include <string.h>
#include <stdbool.h>

/*
 Source:
 http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
*/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static void print_test_behavior(char *description, char **p, char *buffer, size_t bufSize);

static bool evaluate_test(lex_token expected_token,
			  char *expected_value,
			  char *expected_remaining, lex_token actual_token, char *actual_value, char *actual_remaining);

static bool test(char *description,
		 char *input_str, lex_token expected_token, char *expected_value, char *expected_remaining);


int main()
{

    int failures = 0, successes = 0;

    test("return no results found for empty path", "", LEX_NOT_FOUND, "", "") ? successes++ : failures++;

    test("parse root node operator", "$.nodename", LEX_ROOT, "", ".nodename") ? successes++ : failures++;

    test("parse current node operator", "@.nodename", LEX_CUR_NODE, "", ".nodename") ? successes++ : failures++;

    test("parse node operator in dot notation and stop at next node operator",
	 ".nodename.nextnode", LEX_NODE, "nodename", ".nextnode") ? successes++ : failures++;

    test("parse node operator in dot notation and stop at space",
	 ".nodename ", LEX_NODE, "nodename", " ") ? successes++ : failures++;

    test("parse node operator in dot notation and stop at bracket",
	 ".nodename[", LEX_NODE, "nodename", "[") ? successes++ : failures++;

    test("parse root node operator", "$.nodename[?()]", LEX_ROOT, "", ".nodename[?()]") ? successes++ : failures++;

    test("parse node operator in bracket notation and single quotes",
	 "['nodename']['nextnode']", LEX_NODE, "nodename", "['nextnode']") ? successes++ : failures++;

    test("parse node operator in bracket notation and single quotes and ignore extra node name operator",
	 ".['nodename']['nextnode']", LEX_NODE, "nodename", "['nextnode']") ? successes++ : failures++;

    test("parse node operator in bracket notation and double quotes",
	 "[\"nodename\"][\"nextnode\"]", LEX_NODE, "nodename", "[\"nextnode\"]") ? successes++ : failures++;

    test("parse deep scan (recursive) operator in dot notation",
	 "..nodename.nodename", LEX_DEEP_SCAN, "", ".nodename.nodename") ? successes++ : failures++;

    test("parse deep scan (recursive) operator in bracket notation",
	 "..['nodename']..['nodename']", LEX_DEEP_SCAN, "", ".['nodename']..['nodename']") ? successes++ : failures++;

    test("parse wild card operator", "*.nodeName", LEX_WILD_CARD, "", ".nodeName") ? successes++ : failures++;

    test("parse an expression terminator operator",
	 "].nodename", LEX_EXPR_END, "", ".nodename") ? successes++ : failures++;

    test("parse an expression slice operator", ":.nodename", LEX_SLICE, "", ".nodename") ? successes++ : failures++;

    test("parse an expression child separator operator",
	 ",.nodename", LEX_CHILD_SEP, "", ".nodename") ? successes++ : failures++;

    test("parse the start of an expression", "[?()]", LEX_EXPR_START, "", "()]") ? successes++ : failures++;

    test("parse a string literal in single quotes",
	 "'here be an expression'==", LEX_LITERAL, "here be an expression", "==") ? successes++ : failures++;

    test("parse a string literal in single quotes with single quotes inside",
	 "'here be 'an' expression'==", LEX_LITERAL, "here be 'an' expression", "==") ? successes++ : failures++;

    test("parse a string literal in single quotes with double quotes inside",
	 "'here be \"an\" expression'==", LEX_LITERAL, "here be \"an\" expression", "==") ? successes++ : failures++;

    test("parse a string literal in double quotes",
	 "\"here be an expression\"==", LEX_LITERAL, "here be an expression", "==") ? successes++ : failures++;

    test("parse a string literal in double quotes with single quotes inside",
	 "\"here be 'an' expression\"==", LEX_LITERAL, "here be 'an' expression", "==") ? successes++ : failures++;

    test("parse a string literal in double quotes with double quotes inside",
	 "\"here be \\\"an\\\" expression\"==", LEX_LITERAL, "here be \\\"an\\\" expression",
	 "==") ? successes++ : failures++;

    test("parse an equals operator", "== .nodename", LEX_EQ, "", " .nodename") ? successes++ : failures++;

    test("parse a not equals operator", "!= .nodename", LEX_NEQ, "", " .nodename") ? successes++ : failures++;

    test("parse a less-than operator", "< .nodename", LEX_LT, "", " .nodename") ? successes++ : failures++;

    test("parse a less-than-or-equals-to operator",
	 "<= .nodename", LEX_LTE, "", " .nodename") ? successes++ : failures++;

    test("parse a less-than operator", "> .nodename", LEX_GT, "", " .nodename") ? successes++ : failures++;

    test("parse a greater-than-or-equals-to operator",
	 ">= .nodename", LEX_GTE, "", " .nodename") ? successes++ : failures++;

    test("parse a regex operator", "=~ .nodename", LEX_RGXP, "", " .nodename") ? successes++ : failures++;

    test("parse an the filter start operator for array slice or index selection",
	 "[100:200]", LEX_FILTER_START, "", "100:200]") ? successes++ : failures++;

    test("parse an expression paren open operator",
	 "(.nodename)", LEX_PAREN_OPEN, "", ".nodename)") ? successes++ : failures++;

    test("parse an expression paren open operator",
	 ") && .nodename", LEX_PAREN_CLOSE, "", " && .nodename") ? successes++ : failures++;

    test("parse a unquoted numeric string literal",
	 "11425345] @.num", LEX_LITERAL, "11425345", "] @.num") ? successes++ : failures++;

    test("parse an AND operator", "&& @.num", LEX_AND, "", " @.num") ? successes++ : failures++;

    test("parse an OR operator", "|| @.num", LEX_OR, "", " @.num") ? successes++ : failures++;

    test("parse a WILDCARD operator", ".*||", LEX_WILD_CARD, "", "||") ? successes++ : failures++;

    test("parse an underscore nodename", ".node_name ||", LEX_NODE, "node_name", " ||") ? successes++ : failures++;

    test("parse a dash nodename", ".node-name ||", LEX_NODE, "node-name", " ||") ? successes++ : failures++;

    printf("\n--------------------\n\n");
    printf("%d test(s) executed\n", successes + failures);
    printf("Success:\t%d\n", successes);
    printf("Failures:\t%d\n\n", failures);

    return 0;
}



void print_test_behavior(char *description, char **p, char *buffer, size_t bufSize)
{
    printf("\n--------------------\n\n");
    printf("scan()\n\n");
    printf("With parameters:\n");
    printf("\t- %s%s\n\t- %s%s\n\t- %d\n\n",
	   *p, strlen(*p) > 0 ? "" : "(Empty)", buffer, strlen(buffer) > 0 ? "" : "(Empty)", (int) bufSize);
    printf("Should:\n");
    printf("\t%s\n\n", description);
}

bool evaluate_test(lex_token expected_token,
		   char *expected_value,
		   char *expected_remaining, lex_token actual_token, char *actual_value, char *actual_remaining)
{
    printf(ANSI_COLOR_BLUE "Expected:\n" ANSI_COLOR_RESET);
    printf("\ttoken\t%s\n"
	   "\tvalue\t'%s%s'\n"
	   "\tremain\t'%s%s'\n\n",
	   visible[expected_token],
	   expected_value, strlen(expected_value) > 0 ? "" : "(Empty)",
	   expected_remaining, strlen(expected_remaining) > 0 ? "" : "(Empty)");
    printf(ANSI_COLOR_BLUE "Actual:\n" ANSI_COLOR_RESET);
    printf("\ttoken\t%s\n"
	   "\tvalue\t'%s%s'\n"
	   "\tremain\t'%s%s'\n\n",
	   visible[actual_token],
	   actual_value, strlen(actual_value) > 0 ? "" : "(Empty)",
	   actual_remaining, strlen(actual_remaining) > 0 ? "" : "(Empty)");
    printf("Result:\n");
    if (strcmp(expected_value, actual_value) == 0
	&& expected_token == actual_token && strcmp(expected_remaining, actual_remaining) == 0) {
	printf(ANSI_COLOR_GREEN "\tSuccess\n" ANSI_COLOR_RESET);
	return true;
    } else {
	printf(ANSI_COLOR_RED "\tFailure\n" ANSI_COLOR_RESET);
	return false;
    }
}

bool test(char *description, char *input_str, lex_token expected_token, char *expected_value, char *expected_remaining)
{

    lex_token actual_token;
    char buffer[1000];
    buffer[0] = '\0';

    print_test_behavior(description, &input_str, buffer, sizeof(buffer)
	);

    actual_token = scan(&input_str, buffer, sizeof(buffer));

    return evaluate_test(expected_token, expected_value, expected_remaining, actual_token, buffer, input_str);
}
