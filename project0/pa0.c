/**********************************************************************
 * Copyright (c) 2019
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

/***********************************************************************
 * parse_command
 *
 * DESCRIPTION
 *	parse @command, and put each command token into @tokens[] and the number of
 *	tokes into @nr_tokens.
 *
 * A command token is defined as a string without any whitespace (i.e., *space*
 * and *tab* in this programming assignment). For exmaple,
 *   command = "  Hello world   Ajou   University!!  "
 *
 * then, nr_tokens = 4 and tokens should be
 *
 *   tokens[0] = "Hello"
 *   tokens[1] = "workd"
 *   tokens[2] = "Ajou"
 *   tokens[3] = "University!!"
 *
 * Another exmaple is;
 *   command = "ls  -al   /home/sanghoon /etc  "
 *
 * then, nr_tokens = 4, and tokens is
 *   tokens[0] = "ls"
 *   tokens[1] = "-al"
 *   tokens[2] = "/home/sanghoon"
 *   tokens[3] = "/etc"
 *
 *
 * RETURN VALUE
 *	Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
int slength(char* str){
	int cnt = 0;
	while(str[cnt] != '\0'){
		cnt++;
	}
	return cnt;
}

bool NullSep(char ch, char *separator)
{
	int size = slength(separator), k;
	if (ch == '\0') return true;
	for (k = 0; k < size; k++)  {
		if (ch == separator[k])  return true;
	}
	return false;
}

char* tokenizer(char* input, char* separator){
	static char *cursor = NULL, *end = NULL;
	char* result = NULL;
	
	if(input != NULL){
		cursor = input;
		end = &cursor[slength(input)]; // 커서 마지막 저장
	}

	if(cursor == NULL || cursor == end) return NULL;

	while(cursor < end && NullSep(*cursor,separator) == true){
		*(cursor++) ='\0'; // 구분자 대신 null 넣기
	}
	result = cursor; // 구분자 아닌곳까지 이동햇으니 구분된 문자열 시작주소 집어넣고 

	while(cursor < end && NullSep(*cursor, separator)== false)
		cursor++; // 구분자 아닌곳은 이동해서 null 만날 때 까지 구분되는 문자열 생성
	*cursor = '\0'; // 다 이동후 문자열의 끝을 지정해줘야 하므로 null 넣기

	if(result == cursor) result = NULL;

	return result;
}

static int parse_command(char *command, int *nr_tokens, char *tokens[])
{
	/* TODO
	 * Followings are example code. You should delete them and implement 
	 * your own code here
	 */

	char* str = NULL;
    int cnt = 0;
    str = tokenizer(command,"\t \n");
	
    while(1){
        
        tokens[cnt++] = str;
		
        str = tokenizer(NULL,"\t \n");
        if(str == NULL) break;
    }
	*nr_tokens = (cnt);
    //for(int i =0;i<cnt;i++) printf("[%d] :tokens %s\n",i,tokens[i]);
    return 0;

}


/***********************************************************************
 * The main function of this program. SHOULD NOT CHANGE THE CODE BELOW
 */
int main(int argc, const char *argv[])
{
	char line[MAX_COMMAND] = { '\0' };
	FILE *input = stdin;

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[1]);
			return -EINVAL;
		}
	}

	while (fgets(line, sizeof(line), input)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens= 0;

		parse_command(line, &nr_tokens, tokens);

		fprintf(stderr, "nr_tokens = %d\n", nr_tokens);
		for (int i = 0; i < nr_tokens; i++) {
			fprintf(stderr, "tokens[%d] = %s\n", i, tokens[i]);
		}
		printf("\n");
	}

	if (input != stdin) fclose(input);

	return 0;
}
