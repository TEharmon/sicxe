/*
* ���ϸ� : my_assembler_20172601.c
* ��  �� : �� ���α׷��� SIC/XE �ӽ��� ���� ������ Assembler ���α׷��� ���η�ƾ����,
* �Էµ� ������ �ڵ� ��, ��ɾ �ش��ϴ� OPCODE�� ã�� ����Ѵ�.
* ���� ������ ���Ǵ� ���ڿ� "00000000"���� �ڽ��� �й��� �����Ѵ�.
*/

/*
*
* ���α׷��� ����� �����Ѵ�.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "my_assembler_20172601.h"
#define _CRT_SECURE_NO_WARNINGS
int totalline = 0;
int pass1save = 0;//pass1���� ������ ��ū���� �ɰ� �����Ҷ� �ݺ�����
int instruction_count;
int copy_size, rdrecsize, wrrecsize;
int outputline; 
int condition_flag = 0; //�����߰� 0�̸� copy 1�̸� RDREC 2�̸� WRREC�� �д� ��
const int mask3 = 4095; //(2^12-1) 3���� ��¿� ����ũ
const int mask4 = 1048575;//(2^20-1) 4���� ��¿� ����ũ
char object_line[300];

//����� ���� ��
				/* ----------------------------------------------------------------------------------
				* ���� : ����ڷ� ���� ����� ������ �޾Ƽ� ��ɾ��� OPCODE�� ã�� ����Ѵ�.
				* �Ű� : ���� ����, ����� ����
				* ��ȯ : ���� = 0, ���� = < 0
				* ���� : ���� ����� ���α׷��� ����Ʈ ������ �����ϴ� ��ƾ�� ������ �ʾҴ�.
				*		   ���� �߰������� �������� �ʴ´�.
				* ----------------------------------------------------------------------------------
				*/
void freeALL();

int main(int args, char *arg[])
{

	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: ���α׷� �ʱ�ȭ�� ���� �߽��ϴ�.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: �н�1 �������� �����Ͽ����ϴ�.  \n");
		return -1;
	}
	//make_opcode_output("output_20172601.txt");
	make_symtab_output("symtab_20172601.txt");
	make_literaltab_output("literaltab_20172601.txt");
	
	if (assem_pass2() < 0)
	{
		printf(" assem_pass2: �н�2 �������� �����Ͽ����ϴ�.  \n");
		return -1;
	}

	make_objectcode_output("output2_20172601.txt");
	
	freeALL();
	return 0;
}

/* ----------------------------------------------------------------------------------
* ���� : ���α׷� �ʱ�ȭ�� ���� �ڷᱸ�� ���� �� ������ �д� �Լ��̴�.
* �Ű� : ����
* ��ȯ : �������� = 0 , ���� �߻� = -1
* ���� : ������ ��ɾ� ���̺��� ���ο� �������� �ʰ� ������ �����ϰ� �ϱ�
*		   ���ؼ� ���� ������ �����Ͽ� ���α׷� �ʱ�ȭ�� ���� ������ �о� �� �� �ֵ���
*		   �����Ͽ���.
* ----------------------------------------------------------------------------------
*/
int init_my_assembler(void)
{
	int result;
	if ((result = init_inst_file("Appendix.txt")) < 0) //inst.data
		return -1;
	if ((result = init_input_file("input.txt")) < 0)
		return -1;
	return result;
}

/* ----------------------------------------------------------------------------------
* ���� : �ӽ��� ���� ��� �ڵ��� ������ �о� ���� ��� ���̺�(inst_table)��
*        �����ϴ� �Լ��̴�.
* �Ű� : ���� ��� ����
* ��ȯ : �������� = 0 , ���� < 0
* ���� : ���� ������� ������ �����Ӱ� �����Ѵ�. ���ô� ������ ����.
*
*	===============================================================================
*		   | �̸� | ���� | ���� �ڵ� | ���۷����� ���� | NULL|
*	===============================================================================
*
* ----------------------------------------------------------------------------------
*/
int init_inst_file(char *inst_file)
{
	FILE *file;
	int errno;
	int i = 0;
	inst in;

	file = fopen(inst_file, "rt");
	if (file != NULL)
	{
		fscanf(file, "%s %d %hu %d", in.str, &(in.format), &(in.op), &(in.ops));
		inst_table[i] = (inst*)malloc(sizeof(inst));
		strcpy(inst_table[i]->str, in.str);
		inst_table[i]->op = in.op;
		inst_table[i]->format = in.format;
		inst_table[i]->ops = in.ops;
		//printf("%s %d %X %d\n", inst_table[i]->str, inst_table[i]->format, inst_table[i]->op, inst_table[i]->ops);
		while (!feof(file))
		{
			++i;
			fscanf(file, "%s %d %hu %d", in.str, &(in.format), &(in.op), &(in.ops));
			inst_table[i] = (inst*)malloc(sizeof(inst));
			strcpy(inst_table[i]->str, in.str);
			inst_table[i]->op = in.op;
			inst_table[i]->format = in.format;
			inst_table[i]->ops = in.ops;
			//printf("%s %d %X %d\n", inst_table[i]->str, inst_table[i]->format, inst_table[i]->op, inst_table[i]->ops);
		}
		fclose(file);
		instruction_count = i;
		errno = 0;
	}
	else
	{
		errno = -1;
	}

	return errno;
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �� �ҽ��ڵ带 �о� �ҽ��ڵ� ���̺�(input_data)�� �����ϴ� �Լ��̴�.
* �Ű� : ������� �ҽ����ϸ�
* ��ȯ : �������� = 0 , ���� < 0
* ���� : ���δ����� �����Ѵ�.
*
* ----------------------------------------------------------------------------------
*/
int init_input_file(char *input_file)
{
	FILE *file;
	int errno;
	int i = 0;
	char linebuffer[100];
	file = fopen("input.txt", "rt");
	if (file != NULL)
	{
		fgets(linebuffer, 100, file);
		input_data[i] = (char*)calloc(strlen(linebuffer), sizeof(char));
		strcpy(input_data[i], linebuffer);
		++totalline;
		while (!feof(file))
		{
			++i;
			fgets(linebuffer, 100, file);
			input_data[i] = (char*)calloc(strlen(linebuffer), sizeof(char));
			strcpy(input_data[i], linebuffer);
			++totalline;
		}
		fclose(file);
		errno = 0;
	}
	else {
		errno = -1;
	}
	return errno;
}

/* ----------------------------------------------------------------------------------
* ���� : �ҽ� �ڵ带 �о�� ��ū������ �м��ϰ� ��ū ���̺��� �ۼ��ϴ� �Լ��̴�.
*        �н� 1�� ���� ȣ��ȴ�.
* �Ű� : �Ľ��� ���ϴ� ���ڿ�
* ��ȯ : �������� = 0 , ���� < 0
* ���� : my_assembler ���α׷������� ���δ����� ��ū �� ������Ʈ ������ �ϰ� �ִ�.
* ----------------------------------------------------------------------------------
*/
int token_parsing(char *str)
{
	char *tok = strtok(str, "\t\n");
	char *tok2 = NULL;
	char tok_s[30];
	int searchindex;
	int searchformat;

	if (pass1save == 0) //ù��° start������ ��츸 ����
	{
		token_table[pass1save] = (token*)malloc(sizeof(token));
		for (int i = 0; i < 4; ++i)
		{
			if (i == 0)
			{
				token_table[pass1save]->label = tok;
			}
			else if (i == 1)
			{
				token_table[pass1save]->operato=tok;
			}
			else if (i == 2)
			{
				strcpy(token_table[pass1save]->operand, tok);

			}
			else
			{
				strcpy(token_table[pass1save]->comment, tok);
			}

			tok = strtok(NULL, "\t\n");
		}
		token_table[pass1save]->nixbpe = 0;

	}
	else
	{
		token_table[pass1save] = (token*)malloc(sizeof(token));
		for (int i = 0; i < 4; ++i)
		{
			if (i == 0) //label
			{
				if (str[0] != '\t')
				{
					token_table[pass1save]->label = tok;
					tok = strtok(NULL, "\t\n");
				}
				else
				{
					token_table[pass1save]->label = "\0";

				}

			}
			else if (i == 1) //operator
			{
				if (tok != NULL)
				{
					token_table[pass1save]->operato=tok;
					tok = strtok(NULL, "\t\n");
				}
				else
				{
					token_table[pass1save]->operato="";
				}
			}
			else if (i == 2)
			{
				//operand ���� �ľ��ϱ�
				if (strcmp(token_table[pass1save]->operato,"RSUB") == 0)
				{
					strcpy(token_table[pass1save]->operand[0], "");
					strcpy(token_table[pass1save]->operand[1], "");
					strcpy(token_table[pass1save]->operand[2], "");
					strcpy(token_table[pass1save]->comment, "");
					//strtok(NULL, "\t\n");
					//continue;
					break;
				}
				int j = 0;
				if (tok != NULL)
				{
					strcpy(tok_s, tok);
					tok2 = strtok(tok_s, ",");
					if (tok2 != NULL)
					{
						strcpy(token_table[pass1save]->operand[0], tok2);
						strcpy(token_table[pass1save]->operand[1], "");
						strcpy(token_table[pass1save]->operand[2], "");
						tok2 = strtok(NULL, ",");

						if (tok2 != NULL)
						{
							strcpy(token_table[pass1save]->operand[1], tok2);
							strcpy(token_table[pass1save]->operand[2], "");
							tok2 = strtok(NULL, ",");
							if (tok2 != NULL)
							{
								strcpy(token_table[pass1save]->operand[2], tok2);
							}
						}
						tok = strtok(NULL, "\t\n");
					}
					else
					{
						strcpy(token_table[pass1save]->operand[0], tok);
						strcpy(token_table[pass1save]->operand[1], "");
						strcpy(token_table[pass1save]->operand[2], "");
						tok = strtok(NULL, "\t\n");
					}
				}
				else
				{
					strcpy(token_table[pass1save]->operand[0], "");
					strcpy(token_table[pass1save]->operand[1], "");
					strcpy(token_table[pass1save]->operand[2], "");
				}

			}
			else //i=4�ϰ��, comment�� �д� �κ�
			{
				if (tok == NULL)
				{
					strcpy(token_table[pass1save]->comment, "\0");
				}
				else
				{
					strcpy(token_table[pass1save]->comment, tok);
				}

				tok = strtok(NULL, "\t\n");
			}

		}
		//nixbpe�� ���ϴ� �˰���
		if (token_table[pass1save]->label != ".")
		{
			if (strchr(token_table[pass1save]->operato,'+')!=NULL) //4������ ����ϴ� ���
			{
				token_table[pass1save]->nixbpe = 49;
				//operand�� X�� ������ nixbpe���� x�� �ش��ϴ� ���� 8 �����ֱ�
				if (strcmp(token_table[pass1save]->operand[1], "X") == 0)
				{
					token_table[pass1save]->nixbpe=token_table[pass1save]->nixbpe+8;
				}
			}
			else //4������ �ƴѰ��
			{
				searchindex = search_opcode(token_table[pass1save]->operato);
				if (searchindex!=-1) //opcode table�� ã�Ƽ� �����ϸ�
				{
					searchformat = inst_table[searchindex]->format;
					if (strchr(token_table[pass1save]->operand[0], '@') != NULL) //���ڸ� �о� @�̸�
					{
						token_table[pass1save]->nixbpe = 34;
					}
					else if (strchr(token_table[pass1save]->operand[0], '#') != NULL)
					{
						token_table[pass1save]->nixbpe = 16;
					}
					else
					{
						if (searchformat == 3 && strcmp(token_table[pass1save]->operato, "RSUB")==0)
							token_table[pass1save]->nixbpe = 48;
						else if (searchformat == 3)
							token_table[pass1save]->nixbpe = 50;
						else
							token_table[pass1save]->nixbpe = 0;
					}
				}
				else //�������� ������
				{
					token_table[pass1save]->nixbpe = 0;
				}
			}
		}
		else 
		{
			token_table[pass1save]->nixbpe = 0;
		}
	}
	++pass1save;
	return 0;
}

/* ----------------------------------------------------------------------------------
* ���� : �Է� ���ڿ��� ���� �ڵ������� �˻��ϴ� �Լ��̴�.
* �Ű� : ��ū ������ ���е� ���ڿ�
* ��ȯ : �������� = ���� ���̺� �ε���, ���� < 0
* ���� :
*
* ----------------------------------------------------------------------------------
*/
int search_opcode(char *str)
{
	//�տ� 4����Ʈ �߰��� +�� ������ �����ϰ� �˻�
	int loc = -1;
	if (str[0] == '+')
	{
		char news[10];
		memset(news, '\0', 10);
		strcpy(news, str);
		char * newstring;
		newstring = strtok(news, "+");
		
		for (int i = 0; i <= instruction_count; ++i)
		{
			if (strcmp(newstring, inst_table[i]->str) == 0)
			{
				loc = i;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i <= instruction_count; ++i)
		{
			if (strcmp(str, inst_table[i]->str) == 0)
			{
				loc = i;
				break;
			}
		}
	}
	return loc;
}

/*
���� �߰� getSymtabAddr
�Է�: ��ū���̺��� ����
���: �Է¿� �´� �ɺ����̺� ����� �ּ�, ����� ���� ������ -1
���: �ɺ����̺��� �ɺ��� �Է¹޾� �װͿ� �ش��ϴ� addr���� �����Ѵ�.
*/
int getSymtabAddr(char *str)
{
	int i;
	if (condition_flag == 0)
		i = 0;
	else if (condition_flag == 1)
		i = 9;
	else
		i = 14;
	int re;
	char inputstr[20];
	char *temp;
	memset(inputstr, '\0', 20);
	strcpy(inputstr, str);
	//@ �� ������ ���
	if (strchr(inputstr, '@') != NULL)
	{
		temp = strtok(inputstr, "@");
		while (i<MAX_LINES)
		{
			if (strcmp(sym_table[i].symbol, temp) == 0)
			{
				re = sym_table[i].addr;
				break;
			}
			++i;
		}
		if (i == MAX_LINES)
		{
			re = -1;
		}
	}
	else
	{
		while (i<MAX_LINES)
		{
			if (strcmp(sym_table[i].symbol, inputstr) == 0 || strstr(sym_table[i].symbol,inputstr)!=NULL)
			{
				re = sym_table[i].addr;
				break;
			}
			++i;
		}
		if (i == MAX_LINES)
		{
			re = -1;
		}
	}
	
	return re;
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �н�1������ �����ϴ� �Լ��̴�.
*		   �н�1������..
*		   1. ���α׷� �ҽ��� ��ĵ�Ͽ� �ش��ϴ� ��ū������ �и��Ͽ� ���α׷� ���κ� ��ū
*		   ���̺��� �����Ѵ�.
*
* �Ű� : ����
* ��ȯ : ���� ���� = 0 , ���� = < 0
* ���� : ���� �ʱ� ���������� ������ ���� �˻縦 ���� �ʰ� �Ѿ �����̴�.
*	  ���� ������ ���� �˻� ��ƾ�� �߰��ؾ� �Ѵ�.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/* input_data�� ���ڿ��� ���پ� �Է� �޾Ƽ�
	* token_parsing()�� ȣ���Ͽ� token_unit�� ����
	*/
	int i = 0;
	int check;
	int tmploc; //�ӽú����� RESB�ϰ�� ���ڿ��ε� operand�� ���ڷ� �ٲ� �����Ѵ�. 
	char *oper1 = NULL; // - ����� ��� �� ����
	char *oper2 = NULL; // ���� ����
	char oper[20];
	char littoken[10];
	char *littoken2;
	int litlocctr[3] = { -1, -1, -1 };
	int litlocctrcount = 0;
	while (i<totalline)
	{
		check = token_parsing(input_data[i]);
		if (check != 0)
		{
			printf("�Ľ��� �����ѵ�..\n");
			break;
		}
		//symboltable & littable ����
		if (i == 0)
		{
			strcpy(sym_table[symtab_index].symbol, token_table[i]->label);
			sym_table[symtab_index].addr = locctr;
			++symtab_index;
		}
		if (i >= 3)
		{
			if (strcmp(token_table[i]->label, ".")!=0) //�� �ƴϸ� ����
			{
				if (token_table[i]->label != "\0") //label�� �ɺ����̺� �����ϴ� ���
				{
					if (strcmp(token_table[i]->label, token_table[2]->operand[0]) == 0) //RDREC�� ���� ���
					{
						//copy_size = locctr;
						locctr = 0;
						strcpy(sym_table[symtab_index].symbol, "RDREC");
						sym_table[symtab_index].addr = locctr;
						++symtab_index;
					}
					else if (strcmp(token_table[i]->label, token_table[2]->operand[1]) == 0) //WRREC�� �������
					{
						rdrecsize = locctr;
						locctr = 0;
						strcpy(sym_table[symtab_index].symbol, "WRREC");
						sym_table[symtab_index].addr = locctr;
						++symtab_index;
					}
					else
					{
						strcpy(sym_table[symtab_index].symbol, token_table[i]->label);
						sym_table[symtab_index].addr = locctr;
						++symtab_index;
						//locctr ������Ű��
						if (token_table[i]->nixbpe >= 0 && token_table[i]->nixbpe % 2 == 1)
						{
							locctr = locctr + 4;
						}
						else if (strcmp(token_table[i]->operato, "RESW") == 0)
						{
							tmploc = atoi(token_table[i]->operand[0]);
							locctr += 3 * tmploc;
						}
						else if (strcmp(token_table[i]->operato, "RESB") == 0)
						{
							tmploc = atoi(token_table[i]->operand[0]);
							locctr += tmploc;
						}
						else if (strcmp(token_table[i]->operato, "WORD") == 0)
						{
							locctr += 3;
						}
						else if (strcmp(token_table[i]->operato, "BYTE") == 0)
						{
							tmploc = (strlen(token_table[i]->operand[0]) - 3) / 2;
							locctr += tmploc;
						}
						else if (search_opcode(token_table[i]->operato) != -1)
						{
							locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						}
						else
						{
							//��Ÿ �ٸ� ���� LTORG, EQU, EXTREF�� ���� ��� �ּҰ��
							//operato==EQU, operand=="*"�� ���
							if ((strcmp(token_table[i]->operato, "EQU") == 0) && (strcmp(token_table[i]->operand[0], "*") == 0))
							{
								copy_size = locctr;
							}
							else if ((strcmp(token_table[i]->operato, "EQU") == 0) && (strchr(token_table[i]->operand[0], '-') != NULL))
							{
								--symtab_index;
								memset(oper, '\0', 20);
								strcpy(oper, token_table[i]->operand[0]);
								oper1 = strtok(oper, "-");
								oper2 = strtok(NULL, "-");
								locctr = getSymtabAddr(oper1) - getSymtabAddr(oper2);
								sym_table[symtab_index].addr = locctr;
								++symtab_index;
							}
							else 
							{

							}
						}
					}

				}
				else //���� ���� �ɺ����̺� �߰����� �ʴ� ��� locctr�� �����
				{
					//locctr ������Ű��
					if (token_table[i]->nixbpe >= 0 && token_table[i]->nixbpe % 2 == 1)
					{
						locctr = locctr + 4;
					}
					else if (strcmp(token_table[i]->operato, "RESW") == 0)
					{
						tmploc = atoi(token_table[i]->operand[0]);
						locctr += 3 * tmploc;
					}
					else if (strcmp(token_table[i]->operato, "RESB") == 0)
					{
						tmploc = atoi(token_table[i]->operand[0]);
						locctr += tmploc;
					}
					else if (strcmp(token_table[i]->operato, "WORD") == 0)
					{
						locctr += 3;
					}
					else if (strcmp(token_table[i]->operato, "BYTE") == 0)
					{
						tmploc = (strlen(token_table[i]->operand[0]) - 3) / 2;
						locctr += tmploc;
					}
					else if (search_opcode(token_table[i]->operato) != -1)
					{
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
					}
					else
					{
						if (strcmp(token_table[i]->operato, "LTORG") == 0)
						{
							int k = 0;
							while (literal_table[k].literal != NULL)
							{
								literal_table[k].addr = locctr;
								locctr += 3;
								++k;
							}
							//literal_table[littab_index].addr = locctr;
							//locctr += strlen(literal_table[littab_index].literal);
						}
						//��Ÿ �ٸ� ���� LTORG, EQU, EXTREF�� ���� ��� �ּҰ��
						else if (strcmp(token_table[i]->operato, "END") == 0)
						{
							literal_table[littab_index].addr = locctr;
							locctr += strlen(literal_table[littab_index].literal)/2;
							wrrecsize = locctr;
							break;
						}
					}
				}
				//operand�� �о� ���ͷ����� �ƴ��� Ȯ���ϴ� ����
				if (strchr(token_table[i]->operand[0], '=') != NULL)
				{
					//literal table�� �˻��غ���, ������ ���� ������ �ǳʶٱ�
					memset(littoken, '\0', 10);
					strcpy(littoken, token_table[i]->operand[0]);
					if (token_table[i]->operand[0][2]=='\0')
					{
						littoken2 = strtok(littoken, "=");
						if (littab_index == -1)
						{
							//ó���̴ϱ� ����
							++littab_index;
							literal_table[littab_index].literal = (char*)malloc(sizeof(char)*strlen(littoken2) + 1);
							strcpy(literal_table[littab_index].literal, littoken2);
							//litlocctr[litlocctrcount] = locctr;
							//++litlocctrcount;
							//literal_table[littab_index].addr = locctr;
						}
						else
						{
							if (isStoredLit(littoken2) == -1)
							{
								++littab_index;
								//insertLIT(littoken2);
								literal_table[littab_index].literal = (char*)malloc(sizeof(char)*strlen(littoken2) + 1);
								strcpy(literal_table[littab_index].literal, littoken2);
								//litlocctr[litlocctrcount] = locctr;
								//++litlocctrcount;
							}
						}
					}
					else
					{
						strtok(littoken, "\'");
						littoken2 = strtok(NULL, "\'");
						if (littab_index == -1)
						{
							//ó���̴ϱ� ����
							++littab_index;
							literal_table[littab_index].literal = (char*)malloc(sizeof(char)*strlen(littoken2) + 1);
							strcpy(literal_table[littab_index].literal, littoken2);
							//litlocctr[litlocctrcount] = locctr;
							//++litlocctrcount;
						}
						else
						{
							if (isStoredLit(littoken2) == -1)
							{
								++littab_index;
								//insertLIT(littoken2);
								literal_table[littab_index].literal = (char*)malloc(sizeof(char)*strlen(littoken2) + 1);
								strcpy(literal_table[littab_index].literal, littoken2);
								//litlocctr[litlocctrcount] = locctr;
								//++litlocctrcount;
							}
						}
					}	

				}
			}
			else {} //.�̸� �׳� ����
		}
		++i;
	}
	outputline = i;
	return 0;
}


/*���ͷ����̺� ����Ǿ��ִ��� Ȯ���ϴ� �Լ�
�Է�: ���ڿ�
���: ���ͷ��� ��ġ ������ -1*/
int isStoredLit(char *str)
{
	int i = 0;
	int re;
	while (i <= littab_index)
	{
		if (strcmp(literal_table[i].literal, str)==0)
		{
			re = i;
			break;
		}
		++i;
	}
	if (i > littab_index)
		re=-1;
	return re;
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ ��ɾ� ���� OPCODE�� ��ϵ� ǥ(���� 3��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*        ���� ���� 3�������� ���̴� �Լ��̹Ƿ� ������ ������Ʈ������ ������ �ʴ´�.
* -----------------------------------------------------------------------------------
*/
void make_opcode_output(char *file_name)
{
	//label	operator	operand	opcode ������ ���Ͽ� �ۼ�
	FILE * file;
	file = fopen(file_name, "wt");
	int i = 0;
	int check; // -1�̸� optable�� ���� ��

	if (file != NULL)
	{
		fprintf(file, "%s\t%s\t\t%d\n", token_table[i]->label, token_table[i]->operato, i);
		++i;
		while (i <= outputline)
		{
			if (strcmp(token_table[i]->label, ".") == 0)
			{
				fprintf(file, "\n");
			}
			else
			{
				check = search_opcode(token_table[i]->operato);
				if (check == -1) //optable �� ���� ���
				{
					if (strcmp(token_table[i]->operand[0], "") == 0)//operand 0
					{
						fprintf(file, "%s\t%s\n", token_table[i]->label, token_table[i]->operato);
					}
					else if ((strcmp(token_table[i]->operand[0], "") != 0) && (strcmp(token_table[i]->operand[1], "") == 0))
					{
						fprintf(file, "%s\t%s\t%s\t\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0]);
					}
					else if ((strcmp(token_table[i]->operand[1], "") != 0) && (strcmp(token_table[i]->operand[2], "") == 0))//operand 2
					{
						fprintf(file, "%s\t%s\t%s"",""%s\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0], token_table[i]->operand[1]);
					}
					else
					{
						fprintf(file, "%s\t%s\t%s"",""%s"",""%s\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0], token_table[i]->operand[1], token_table[i]->operand[2]);
					}

				}
				else //�ִ°��
				{
					//operand�� ������ ���� �������� ������ ���
					if (inst_table[check]->ops == 1) //���ڰ� 1��
					{
						fprintf(file, "%s\t%s\t%s\t%x\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0], inst_table[check]->op);
					}
					else if (inst_table[check]->ops == 2)
					{
						fprintf(file, "%s\t%s\t%s"",""%s\t%x\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0], token_table[i]->operand[1], inst_table[check]->op);
					}
					else if (inst_table[check]->ops == 3)
					{
						fprintf(file, "%s\t%s\t%s"",""%s"",""%s\t%x\n", token_table[i]->label, token_table[i]->operato,token_table[i]->operand[0], token_table[i]->operand[1], token_table[i]->operand[2], inst_table[check]->op);
					}
					else
					{
						if (strcmp(token_table[i]->operato,"RSUB") == 0)
						{
							fprintf(file, "\t%s\t\t%x\n", token_table[i]->operato, inst_table[check]->op);
						}
						else
						{
							fprintf(file, "%s\t%s\t\t%x\n", token_table[i]->label, token_table[i]->operato,inst_table[check]->op);
						}

					}
				}
			}

			++i;
		}
		fclose(file);
	}
	printf("�Ϸ�\n");
}

void freeALL() //�޸𸮿� �Ҵ� ���� ��� ���̺� �Ҵ�����
{
	int i;
	for (i = 0; i <= instruction_count; i++)
	{
		free(inst_table[i]);
	}
	for (i = 0; i < totalline; i++)
	{
		input_data[i] = NULL;
	}
	for (i = 0; i <= MAX_LINES; i++)
	{
		free(token_table[i]);
	}
	for (i = 0; i <= littab_index; i++)
	{
		free(literal_table[i].literal);
	}
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ SYMBOL�� �ּҰ��� ����� TABLE�̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------*/
void make_symtab_output(char *file_name)
{
	int i = 0;
	if (file_name != NULL)
	{
		FILE *file;
		file = fopen(file_name, "wt");
		printf("\n=====SymbolTable=====\n");
		for (i = 0; i < symtab_index; i++)
		{
			printf("%s\t%4X\n", sym_table[i].symbol, sym_table[i].addr);
			fprintf(file, "%s\t%4X\n", sym_table[i].symbol, sym_table[i].addr);
		}
		fclose(file);
	}
	else
	{
		printf("\n=====SymbolTable=====\n");
		for (i = 0; i < symtab_index; i++)
		{
			printf("%s\t%4X\n", sym_table[i].symbol, sym_table[i].addr);
		}
	}
	
}



/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ LITERAL�� �ּҰ��� ����� TABLE�̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------*/
void make_literaltab_output(char *file_name)
{
	int i = 0;
	if (file_name != NULL)
	{
		FILE *file;
		file = fopen(file_name, "wt");
		printf("\n=====Literal Table=====\n");
		for (i = 0; i <= littab_index; i++)
		{
			printf("%s\t%4X\n", literal_table[i].literal, literal_table[i].addr);
			fprintf(file, "%s\t%4X\n", literal_table[i].literal, literal_table[i].addr);
		}
		fclose(file);
	}
	else
	{
		printf("\n=====Literal Table=====\n");
		for (i = 0; i <= littab_index; i++)
		{
			printf("%s\t%4X\n", literal_table[i].literal, literal_table[i].addr);
		}
	}
	
}

/*
�����߰� getLittabAddr
�Է�: ��ū ���̺��� operand
���: �Է¿� �´� Iiteral�� �ּ�, ������ -1*/
int getLittabAddr(char *str)
{
	int i = 0;
	int re;
	char inputstr[20];
	strcpy(inputstr, str);
	while (i <= littab_index)
	{
		if (strstr(inputstr, literal_table[i].literal) != NULL)
		{
			re = literal_table[i].addr;
			break;
		}
		++i;
	}
	if (i > littab_index)
		re = -1;
	return re;
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �ڵ�� �ٲٱ� ���� �н�2 ������ �����ϴ� �Լ��̴�.
*		   �н� 2������ ���α׷��� ����� �ٲٴ� �۾��� ���� ������ ����ȴ�.
*		   ������ ���� �۾��� ����Ǿ� ����.
*		   1. ������ �ش� ����� ��ɾ ����� �ٲٴ� �۾��� �����Ѵ�.
* �Ű� : ����
* ��ȯ : �������� = 0, �����߻� = < 0
* ���� :
* -----------------------------------------------------------------------------------*/
static int assem_pass2(void) //operator �� operand�� �о� �ν�Ʈ���� �ڵ带 �ϼ��Ѵ�.
{
	int i = 0;
	unsigned char opcode;
	short xbpe=0;
	int operand_address;
	char instbuffer[10];
	int r1;
	int	r2; //2���� �������� ��¿�
	memset(object_line, '\0', 300);
	int line_acumal = 0; //������Ʈ �ڵ� ���δ� ���� ����Ʈ ���� ����

	Modify_table m[20];
	int modifyindex = 0;

	while (i <= outputline)
	{
		if (i == 0)
			strcpy(object_line," ");
		operand_address = 0;
		memset(instbuffer, '\0', 10);
		r1 = r2 = 0;
		if (strcmp(token_table[i]->label, ".") != 0) //label�� . �� �ƴ� ���
		{
			if (strcmp(token_table[i]->operato, "START") == 0)
			{
				locctr = 0;
				printf("\nH%s%06X%06X\n",token_table[i]->label,locctr,copy_size);
			}
			else if (strcmp(token_table[i]->operato, "END") == 0)
			{
				//printf("%c%c\n", literal_table[1].literal[0], literal_table[1].literal[1]);
				sprintf(instbuffer,"%c%c\n", literal_table[1].literal[0], literal_table[1].literal[1]);
				strcat(object_line, instbuffer);
				locctr =locctr+ strlen(literal_table[littab_index].literal)-1;
				line_acumal=line_acumal+ strlen(literal_table[littab_index].literal) - 1;
				printf("%02X%s\n", line_acumal, object_line);
				memset(object_line, '\0', 300);
				strcpy(object_line, " ");
				//printf("\nM%06X%02X%s\n");
				for (int j = 0; j < modifyindex; j++)
				{
					if (condition_flag == m[j].condition && m[j].toFix == 5) {
						printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
					}
					else if (condition_flag == m[j].condition && m[j].toFix == 6)
					{
						printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
						printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
					}
					else {}
				}
				printf("E\n");
				break;
			}
			else if (strcmp(token_table[i]->operato, "EXTDEF") == 0)
			{
				printf("D%s%06X%s%06X%s%06X\n",token_table[i]->operand[0],getSymtabAddr(token_table[i]->operand[0])
				, token_table[i]->operand[1], getSymtabAddr(token_table[i]->operand[1])
				, token_table[i]->operand[2], getSymtabAddr(token_table[i]->operand[2]));
			}
			else if (strcmp(token_table[i]->operato, "EXTREF") == 0)
			{
				printf("R%s", token_table[i]->operand[0]);
				if (strcmp(token_table[i]->operand[1], "") != 0)
					printf("%s", token_table[i]->operand[1]);
				if (strcmp(token_table[i]->operand[2], "") != 0)
					printf("%s", token_table[i]->operand[2]);
				printf("\nT%06X",locctr);
			}
			else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "RDREC") == 0)
			{
				
				line_acumal = 0;
				locctr = 0;
				//printf("\nM%06X%02X%s\n");
				for (int j = 0; j < modifyindex; j++)
				{
					if (condition_flag == m[j].condition && m[j].toFix==5) {
						printf("M%06X%02X+%s\n",m[j].addr,m[j].toFix,m[j].operand);
					}
					else if (condition_flag == m[j].condition && m[j].toFix == 6)
					{
						printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand,"-"));
						printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
					}
					else {}
				}
				printf("E%06X\n", locctr);
				printf("\nH%s%06X%06X\n",token_table[i]->label,locctr,rdrecsize);
				++condition_flag;
			}
			else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "WRREC") == 0)
			{
				printf("%02X%s\n", line_acumal, object_line);
				memset(object_line, '\0', 300);
				strcpy(object_line, " ");
				locctr = 0;
				line_acumal = 0;
				//printf("\nM%06X%02X%s\n");
				for (int j = 0; j < modifyindex; j++)
				{
					if (condition_flag == m[j].condition && m[j].toFix == 5) {
						printf("\nM%06X%02X+%s", m[j].addr, m[j].toFix, m[j].operand);
					}
					else if (condition_flag == m[j].condition && m[j].toFix == 6)
					{
						printf("\nM%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
						printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
					}
					else {}
				}
				printf("E\n");
				printf("\nH%s%06X%06X\n", token_table[i]->label, locctr, wrrecsize);
				++condition_flag;
			}
			else if (strcmp(token_table[i]->operato, "RESW") == 0)
			{
				locctr += 3 * atoi(token_table[i]->operand[0]);
			}
			else if (strcmp(token_table[i]->operato, "RESB") == 0)
			{
				locctr += atoi(token_table[i]->operand[0]);
			}
			else if (strcmp(token_table[i]->operato, "WORD") == 0)
			{
				if (line_acumal >= 29)
				{
					printf("%02X%s\n", line_acumal, object_line);
					line_acumal = 0;
					printf("T%06X", locctr);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
				}
				m[modifyindex].addr = locctr;
				m[modifyindex].condition = condition_flag;
				m[modifyindex].toFix = 6;
				strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
				++modifyindex;
				locctr += 3;
				line_acumal += 3;
				//printf("%06d",operand_address);
				sprintf(instbuffer, "%06d", operand_address);
				strcat(object_line, instbuffer);
			}
			else if (strcmp(token_table[i]->operato, "BYTE") == 0)
			{
				if (line_acumal >= 29)
				{
					printf("%02X%s\n", line_acumal, object_line);
					line_acumal = 0;
					printf("T%06X", locctr);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
				}
				//printf("%c%c",token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
				sprintf(instbuffer, "%c%c", token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
				strcat(object_line,instbuffer);
				locctr += (strlen(token_table[i]->operand[0]) - 3) / 2;
				line_acumal+= (strlen(token_table[i]->operand[0]) - 3) / 2;
			}
			else if (strcmp(token_table[i]->operato, "LTORG") == 0)
			{
				printf("%02X%s\n", line_acumal, object_line);
				line_acumal = 0;
				memset(object_line, '\0', 300);
				strcpy(object_line, " ");
				printf("\nT%06X",locctr);
				printf("%02X",strlen(literal_table[0].literal));
				printf("%X""%X""%X\n",literal_table[0].literal[0], literal_table[0].literal[1], literal_table[0].literal[2]);
				
				locctr += strlen(literal_table[0].literal);
				
			}
			else
			{
				if (token_table[i]->nixbpe % 2 == 1 && token_table[i]->nixbpe >= 48) //4����
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n",line_acumal,object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					m[modifyindex].addr = locctr + 1;
					m[modifyindex].toFix = 5;
					m[modifyindex].condition = condition_flag;
					strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
					++modifyindex;
					locctr += 4;
					line_acumal += 4;
					opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
					xbpe = token_table[i]->nixbpe - 48;
					operand_address = 0;
					
					//printf("%02X""%hhd""%05X", opcode, xbpe, (operand_address)&mask4);
					sprintf(instbuffer, "%X""%hhd""05X", opcode, xbpe, (operand_address)&mask4);
					strcat(object_line, instbuffer);
				}
				else if (token_table[i]->nixbpe >= 48 && token_table[i]->nixbpe % 2 == 0) //sicxe �ܼ� ��巹��
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					locctr += 3;
					line_acumal += 3;
					opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
					xbpe = token_table[i]->nixbpe - 48;
					if (xbpe==2)
					{
						operand_address = getSymtabAddr(token_table[i]->operand[0]);
						if (operand_address == -1)
						{
							operand_address = getLittabAddr(token_table[i]->operand[0]);
							if (operand_address == -1)
							{
								operand_address = 0;
								//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								strcat(object_line, instbuffer);
							}
							else
							{
								operand_address = operand_address - locctr;
								
								//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								strcat(object_line, instbuffer);
							}					
						}
						else
						{
							operand_address = operand_address - locctr;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
						
					}
					else
					{
						operand_address = 0;
						//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						sprintf(instbuffer,"%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						strcat(object_line,instbuffer);
					}
				}
				else if (token_table[i]->nixbpe < 48 && token_table[i]->nixbpe >= 32) //indirect ��巹�� @operand
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
					line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

					opcode = inst_table[search_opcode(token_table[i]->operato)]->op+2;
					xbpe = token_table[i]->nixbpe - 32;
					//operand_address���ϱ�
					if (xbpe == 2)
					{
						operand_address = getSymtabAddr(token_table[i]->operand[0]);
						operand_address = operand_address - locctr;
						//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						sprintf(instbuffer,"%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						strcat(object_line, instbuffer);
					}
					else
					{
						operand_address = 0;
						//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						strcat(object_line, instbuffer);
					}
				}
				else if (token_table[i]->nixbpe < 32 && token_table[i]->nixbpe >= 16) //immediate ��巹�� #operand
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
					line_acumal+= inst_table[search_opcode(token_table[i]->operato)]->format;

					opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 1;
					xbpe= token_table[i]->nixbpe - 16;
					operand_address =token_table[i]->operand[0][1]-48;
					//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
					sprintf(instbuffer,"%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
					strcat(object_line, instbuffer);
				}
				else if (search_opcode(token_table[i]->operato) != -1 && inst_table[search_opcode(token_table[i]->operato)]->format < 3)//2����
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					locctr+= inst_table[search_opcode(token_table[i]->operato)]->format;
					line_acumal+= inst_table[search_opcode(token_table[i]->operato)]->format;

					opcode = inst_table[search_opcode(token_table[i]->operato)]->op;
					//A:0, S:4, T:5, X:1
					if (inst_table[search_opcode(token_table[i]->operato)]->ops == 1)//register 1��
					{
						if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
						else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
						else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
						else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
						else {}
						//printf("%02X%X%X",opcode,r1,r2);
						sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
						strcat(object_line, instbuffer);
					}
					else //register 2��
					{
						if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
						else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
						else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
						else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
						else {}
						if (strcmp(token_table[i]->operand[1], "A") == 0) { r2 = 0; }
						else if (strcmp(token_table[i]->operand[1], "S") == 0) { r2 = 4; }
						else if (strcmp(token_table[i]->operand[1], "T") == 0) { r2 = 5; }
						else if (strcmp(token_table[i]->operand[1], "X") == 0) { r2 = 1; }
						else {}
						//printf("%X%X%X", opcode, r1, r2);
						sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
						strcat(object_line, instbuffer);
					}
				}
			}
		}
		else // .�� ���� ����
		{

		}
		++i;
	}
	condition_flag = 0;
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ object code (������Ʈ 1��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------*/
void make_objectcode_output(char *file_name)
{
	if (file_name != NULL)
	{
		FILE *file;
		file = fopen(file_name, "wt");
		/* add your code here */
		int i = 0;
		unsigned char opcode;
		short xbpe = 0;
		int operand_address;
		char instbuffer[10];
		int r1;
		int	r2; //2���� �������� ��¿�
		memset(object_line, '\0', 300);
		int line_acumal = 0; //������Ʈ �ڵ� ���δ� ���� ����Ʈ ���� ����

		Modify_table m[20];
		int modifyindex = 0;

		while (i <= outputline)
		{
			if (i == 0)
				strcpy(object_line, " ");
			operand_address = 0;
			memset(instbuffer, '\0', 10);
			r1 = r2 = 0;
			if (strcmp(token_table[i]->label, ".") != 0) //label�� . �� �ƴ� ���
			{
				if (strcmp(token_table[i]->operato, "START") == 0)
				{
					locctr = 0;
					fprintf(file,"H%s%06X%06X\n", token_table[i]->label, locctr, copy_size);
				}
				else if (strcmp(token_table[i]->operato, "END") == 0)
				{
					//printf("%c%c\n", literal_table[1].literal[0], literal_table[1].literal[1]);
					sprintf(instbuffer, "%c%c", literal_table[1].literal[0], literal_table[1].literal[1]);
					strcat(object_line, instbuffer);
					locctr = locctr + strlen(literal_table[littab_index].literal) - 1;
					line_acumal = line_acumal + strlen(literal_table[littab_index].literal) - 1;
					fprintf(file,"%02X%s\n", line_acumal, object_line);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							fprintf(file,"M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					fprintf(file,"E\n");
					break;
				}
				else if (strcmp(token_table[i]->operato, "EXTDEF") == 0)
				{
					fprintf(file,"D%s%06X%s%06X%s%06X\n", token_table[i]->operand[0], getSymtabAddr(token_table[i]->operand[0])
						, token_table[i]->operand[1], getSymtabAddr(token_table[i]->operand[1])
						, token_table[i]->operand[2], getSymtabAddr(token_table[i]->operand[2]));
				}
				else if (strcmp(token_table[i]->operato, "EXTREF") == 0)
				{
					fprintf(file,"R%s", token_table[i]->operand[0]);
					if (strcmp(token_table[i]->operand[1], "") != 0)
						fprintf(file,"%s", token_table[i]->operand[1]);
					if (strcmp(token_table[i]->operand[2], "") != 0)
						fprintf(file,"%s", token_table[i]->operand[2]);
					fprintf(file,"\nT%06X", locctr);
				}
				else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "RDREC") == 0)
				{

					line_acumal = 0;
					locctr = 0;
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							fprintf(file,"M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					fprintf(file,"E%06X\n", locctr);
					fprintf(file,"H%s%06X%06X\n", token_table[i]->label, locctr, rdrecsize);
					++condition_flag;
				}
				else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "WRREC") == 0)
				{
					fprintf(file,"%02X%s\n", line_acumal, object_line);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					locctr = 0;
					line_acumal = 0;
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							fprintf(file,"M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							fprintf(file,"M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					fprintf(file,"E\n");
					fprintf(file,"H%s%06X%06X\n", token_table[i]->label, locctr, wrrecsize);
					++condition_flag;
				}
				else if (strcmp(token_table[i]->operato, "RESW") == 0)
				{
					locctr += 3 * atoi(token_table[i]->operand[0]);
				}
				else if (strcmp(token_table[i]->operato, "RESB") == 0)
				{
					locctr += atoi(token_table[i]->operand[0]);
				}
				else if (strcmp(token_table[i]->operato, "WORD") == 0)
				{
					if (line_acumal >= 29)
					{
						fprintf(file,"%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						fprintf(file,"T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					m[modifyindex].addr = locctr;
					m[modifyindex].condition = condition_flag;
					m[modifyindex].toFix = 6;
					strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
					++modifyindex;
					locctr += 3;
					line_acumal += 3;
					//printf("%06d",operand_address);
					sprintf(instbuffer, "%06d", operand_address);
					strcat(object_line, instbuffer);
				}
				else if (strcmp(token_table[i]->operato, "BYTE") == 0)
				{
					if (line_acumal >= 29)
					{
						fprintf(file,"%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						fprintf(file,"T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					//printf("%c%c",token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
					sprintf(instbuffer, "%c%c", token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
					strcat(object_line, instbuffer);
					locctr += (strlen(token_table[i]->operand[0]) - 3) / 2;
					line_acumal += (strlen(token_table[i]->operand[0]) - 3) / 2;
				}
				else if (strcmp(token_table[i]->operato, "LTORG") == 0)
				{
					fprintf(file,"%02X%s\n", line_acumal, object_line);
					line_acumal = 0;
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					fprintf(file,"T%06X", locctr);
					fprintf(file,"%02X", strlen(literal_table[0].literal)+2+ strlen(literal_table[1].literal)+2+ strlen(literal_table[2].literal));
					fprintf(file,"%06X",atoi(literal_table[0].literal));
					fprintf(file, "%X""%X""%X", literal_table[1].literal[0], literal_table[1].literal[1], literal_table[1].literal[2]);
					fprintf(file, "%06X\n", atoi(literal_table[2].literal));
					
					locctr += 9;
					//locctr += strlen(literal_table[0].literal);

				}
				else
				{
					if (token_table[i]->nixbpe % 2 == 1 && token_table[i]->nixbpe >= 48) //4����
					{
						if (line_acumal >= 29)
						{
							fprintf(file,"%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							fprintf(file,"T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						m[modifyindex].addr = locctr + 1;
						m[modifyindex].toFix = 5;
						m[modifyindex].condition = condition_flag;
						strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
						++modifyindex;
						locctr += 4;
						line_acumal += 4;
						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
						xbpe = token_table[i]->nixbpe - 48;
						operand_address = 0;

						//printf("%02X""%hhd""%05X", opcode, xbpe, (operand_address)&mask4);
						sprintf(instbuffer, "%X""%hhd""%05X", opcode, xbpe, (operand_address)&mask4);
						strcat(object_line, instbuffer);
					}
					else if (token_table[i]->nixbpe >= 48 && token_table[i]->nixbpe % 2 == 0) //sicxe �ܼ� ��巹��
					{
						if (line_acumal >= 29)
						{
							fprintf(file,"%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							fprintf(file,"T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += 3;
						line_acumal += 3;
						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
						xbpe = token_table[i]->nixbpe - 48;
						if (xbpe == 2)
						{
							operand_address = getSymtabAddr(token_table[i]->operand[0]);
							if (operand_address == -1)
							{
								operand_address = getLittabAddr(token_table[i]->operand[0]);
								if (operand_address == -1)
								{
									operand_address = 0;
									//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									strcat(object_line, instbuffer);
								}
								else
								{
									operand_address = operand_address - locctr;

									//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									strcat(object_line, instbuffer);
								}
							}
							else
							{
								operand_address = operand_address - locctr;
								//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								strcat(object_line, instbuffer);
							}

						}
						else
						{
							operand_address = 0;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
					}
					else if (token_table[i]->nixbpe < 48 && token_table[i]->nixbpe >= 32) //indirect ��巹�� @operand
					{
						if (line_acumal >= 29)
						{
							fprintf(file,"%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							fprintf(file,"T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 2;
						xbpe = token_table[i]->nixbpe - 32;
						//operand_address���ϱ�
						if (xbpe == 2)
						{
							operand_address = getSymtabAddr(token_table[i]->operand[0]);
							operand_address = operand_address - locctr;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
						else
						{
							operand_address = 0;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
					}
					else if (token_table[i]->nixbpe < 32 && token_table[i]->nixbpe >= 16) //immediate ��巹�� #operand
					{
						if (line_acumal >= 29)
						{
							fprintf(file,"%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							fprintf(file,"T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 1;
						xbpe = token_table[i]->nixbpe - 16;
						operand_address = token_table[i]->operand[0][1] - 48;
						//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						strcat(object_line, instbuffer);
					}
					else if (search_opcode(token_table[i]->operato) != -1 && inst_table[search_opcode(token_table[i]->operato)]->format < 3)//2����
					{
						if (line_acumal >= 29)
						{
							fprintf(file,"%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							fprintf(file,"T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op;
						//A:0, S:4, T:5, X:1
						if (inst_table[search_opcode(token_table[i]->operato)]->ops == 1)//register 1��
						{
							if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
							else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
							else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
							else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
							else {}
							//printf("%02X%X%X",opcode,r1,r2);
							sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
							strcat(object_line, instbuffer);
						}
						else //register 2��
						{
							if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
							else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
							else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
							else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
							else {}
							if (strcmp(token_table[i]->operand[1], "A") == 0) { r2 = 0; }
							else if (strcmp(token_table[i]->operand[1], "S") == 0) { r2 = 4; }
							else if (strcmp(token_table[i]->operand[1], "T") == 0) { r2 = 5; }
							else if (strcmp(token_table[i]->operand[1], "X") == 0) { r2 = 1; }
							else {}
							//printf("%X%X%X", opcode, r1, r2);
							sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
							strcat(object_line, instbuffer);
						}
					}
				}
			}
			else // .�� ���� ����
			{

			}
			++i;
		}
		condition_flag = 0;
		fclose(file);
	}
	else
	{
		int i = 0;
		unsigned char opcode;
		short xbpe = 0;
		int operand_address;
		char instbuffer[10];
		int r1;
		int	r2; //2���� �������� ��¿�
		memset(object_line, '\0', 300);
		int line_acumal = 0; //������Ʈ �ڵ� ���δ� ���� ����Ʈ ���� ����

		Modify_table m[20];
		int modifyindex = 0;

		while (i <= outputline)
		{
			if (i == 0)
				strcpy(object_line, " ");
			operand_address = 0;
			memset(instbuffer, '\0', 10);
			r1 = r2 = 0;
			if (strcmp(token_table[i]->label, ".") != 0) //label�� . �� �ƴ� ���
			{
				if (strcmp(token_table[i]->operato, "START") == 0)
				{
					locctr = 0;
					printf("\nH%s%06X%06X\n", token_table[i]->label, locctr, copy_size);
				}
				else if (strcmp(token_table[i]->operato, "END") == 0)
				{
					//printf("%c%c\n", literal_table[1].literal[0], literal_table[1].literal[1]);
					sprintf(instbuffer, "%c%c\n", literal_table[1].literal[0], literal_table[1].literal[1]);
					strcat(object_line, instbuffer);
					locctr = locctr + strlen(literal_table[littab_index].literal) - 1;
					line_acumal = line_acumal + strlen(literal_table[littab_index].literal) - 1;
					printf("%02X%s\n", line_acumal, object_line);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					printf("E\n");
					break;
				}
				else if (strcmp(token_table[i]->operato, "EXTDEF") == 0)
				{
					printf("D%s%06X%s%06X%s%06X\n", token_table[i]->operand[0], getSymtabAddr(token_table[i]->operand[0])
						, token_table[i]->operand[1], getSymtabAddr(token_table[i]->operand[1])
						, token_table[i]->operand[2], getSymtabAddr(token_table[i]->operand[2]));
				}
				else if (strcmp(token_table[i]->operato, "EXTREF") == 0)
				{
					printf("R%s", token_table[i]->operand[0]);
					if (strcmp(token_table[i]->operand[1], "") != 0)
						printf("%s", token_table[i]->operand[1]);
					if (strcmp(token_table[i]->operand[2], "") != 0)
						printf("%s", token_table[i]->operand[2]);
					printf("\nT%06X", locctr);
				}
				else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "RDREC") == 0)
				{

					line_acumal = 0;
					locctr = 0;
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							printf("M%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					printf("E%06X\n", locctr);
					printf("\nH%s%06X%06X\n", token_table[i]->label, locctr, rdrecsize);
					++condition_flag;
				}
				else if (strcmp(token_table[i]->operato, "CSECT") == 0 && strcmp(token_table[i]->label, "WRREC") == 0)
				{
					printf("%02X%s\n", line_acumal, object_line);
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					locctr = 0;
					line_acumal = 0;
					//printf("\nM%06X%02X%s\n");
					for (int j = 0; j < modifyindex; j++)
					{
						if (condition_flag == m[j].condition && m[j].toFix == 5) {
							printf("\nM%06X%02X+%s", m[j].addr, m[j].toFix, m[j].operand);
						}
						else if (condition_flag == m[j].condition && m[j].toFix == 6)
						{
							printf("\nM%06X%02X+%s\n", m[j].addr, m[j].toFix, strtok(m[j].operand, "-"));
							printf("M%06X%02X-%s\n", m[j].addr, m[j].toFix, strtok(NULL, "-"));
						}
						else {}
					}
					printf("E\n");
					printf("\nH%s%06X%06X\n", token_table[i]->label, locctr, wrrecsize);
					++condition_flag;
				}
				else if (strcmp(token_table[i]->operato, "RESW") == 0)
				{
					locctr += 3 * atoi(token_table[i]->operand[0]);
				}
				else if (strcmp(token_table[i]->operato, "RESB") == 0)
				{
					locctr += atoi(token_table[i]->operand[0]);
				}
				else if (strcmp(token_table[i]->operato, "WORD") == 0)
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					m[modifyindex].addr = locctr;
					m[modifyindex].condition = condition_flag;
					m[modifyindex].toFix = 6;
					strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
					++modifyindex;
					locctr += 3;
					line_acumal += 3;
					//printf("%06d",operand_address);
					sprintf(instbuffer, "%06d", operand_address);
					strcat(object_line, instbuffer);
				}
				else if (strcmp(token_table[i]->operato, "BYTE") == 0)
				{
					if (line_acumal >= 29)
					{
						printf("%02X%s\n", line_acumal, object_line);
						line_acumal = 0;
						printf("T%06X", locctr);
						memset(object_line, '\0', 300);
						strcpy(object_line, " ");
					}
					//printf("%c%c",token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
					sprintf(instbuffer, "%c%c", token_table[i]->operand[0][2], token_table[i]->operand[0][3]);
					strcat(object_line, instbuffer);
					locctr += (strlen(token_table[i]->operand[0]) - 3) / 2;
					line_acumal += (strlen(token_table[i]->operand[0]) - 3) / 2;
				}
				else if (strcmp(token_table[i]->operato, "LTORG") == 0)
				{
					printf("%02X%s\n", line_acumal, object_line);
					line_acumal = 0;
					memset(object_line, '\0', 300);
					strcpy(object_line, " ");
					printf("\nT%06X", locctr);
					printf("%02X", strlen(literal_table[0].literal));
					printf("%X""%X""%X\n", literal_table[0].literal[0], literal_table[0].literal[1], literal_table[0].literal[2]);

					locctr += 3;

				}
				else
				{
					if (token_table[i]->nixbpe % 2 == 1 && token_table[i]->nixbpe >= 48) //4����
					{
						if (line_acumal >= 29)
						{
							printf("%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							printf("T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						m[modifyindex].addr = locctr + 1;
						m[modifyindex].toFix = 5;
						m[modifyindex].condition = condition_flag;
						strcpy(m[modifyindex].operand, token_table[i]->operand[0]);
						++modifyindex;
						locctr += 4;
						line_acumal += 4;
						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
						xbpe = token_table[i]->nixbpe - 48;
						operand_address = 0;

						//printf("%02X""%hhd""%05X", opcode, xbpe, (operand_address)&mask4);
						sprintf(instbuffer, "%X""%hhd""%X", opcode, xbpe, (operand_address)&mask4);
						strcat(object_line, instbuffer);
					}
					else if (token_table[i]->nixbpe >= 48 && token_table[i]->nixbpe % 2 == 0) //sicxe �ܼ� ��巹��
					{
						if (line_acumal >= 29)
						{
							printf("%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							printf("T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += 3;
						line_acumal += 3;
						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 3;
						xbpe = token_table[i]->nixbpe - 48;
						if (xbpe == 2)
						{
							operand_address = getSymtabAddr(token_table[i]->operand[0]);
							if (operand_address == -1)
							{
								operand_address = getLittabAddr(token_table[i]->operand[0]);
								if (operand_address == -1)
								{
									operand_address = 0;
									//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									strcat(object_line, instbuffer);
								}
								else
								{
									operand_address = operand_address - locctr;

									//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
									strcat(object_line, instbuffer);
								}
							}
							else
							{
								operand_address = operand_address - locctr;
								//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
								strcat(object_line, instbuffer);
							}

						}
						else
						{
							operand_address = 0;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
					}
					else if (token_table[i]->nixbpe < 48 && token_table[i]->nixbpe >= 32) //indirect ��巹�� @operand
					{
						if (line_acumal >= 29)
						{
							printf("%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							printf("T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 2;
						xbpe = token_table[i]->nixbpe - 32;
						//operand_address���ϱ�
						if (xbpe == 2)
						{
							operand_address = getSymtabAddr(token_table[i]->operand[0]);
							operand_address = operand_address - locctr;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
						else
						{
							operand_address = 0;
							//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
							strcat(object_line, instbuffer);
						}
					}
					else if (token_table[i]->nixbpe < 32 && token_table[i]->nixbpe >= 16) //immediate ��巹�� #operand
					{
						if (line_acumal >= 29)
						{
							printf("%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							printf("T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op + 1;
						xbpe = token_table[i]->nixbpe - 16;
						operand_address = token_table[i]->operand[0][1] - 48;
						//printf("%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						sprintf(instbuffer, "%02X""%hhd""%03X", opcode, xbpe, (operand_address)&mask3);
						strcat(object_line, instbuffer);
					}
					else if (search_opcode(token_table[i]->operato) != -1 && inst_table[search_opcode(token_table[i]->operato)]->format < 3)//2����
					{
						if (line_acumal >= 29)
						{
							printf("%02X%s\n", line_acumal, object_line);
							line_acumal = 0;
							printf("T%06X", locctr);
							memset(object_line, '\0', 300);
							strcpy(object_line, " ");
						}
						locctr += inst_table[search_opcode(token_table[i]->operato)]->format;
						line_acumal += inst_table[search_opcode(token_table[i]->operato)]->format;

						opcode = inst_table[search_opcode(token_table[i]->operato)]->op;
						//A:0, S:4, T:5, X:1
						if (inst_table[search_opcode(token_table[i]->operato)]->ops == 1)//register 1��
						{
							if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
							else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
							else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
							else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
							else {}
							//printf("%02X%X%X",opcode,r1,r2);
							sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
							strcat(object_line, instbuffer);
						}
						else //register 2��
						{
							if (strcmp(token_table[i]->operand[0], "A") == 0) { r1 = 0; }
							else if (strcmp(token_table[i]->operand[0], "S") == 0) { r1 = 4; }
							else if (strcmp(token_table[i]->operand[0], "T") == 0) { r1 = 5; }
							else if (strcmp(token_table[i]->operand[0], "X") == 0) { r1 = 1; }
							else {}
							if (strcmp(token_table[i]->operand[1], "A") == 0) { r2 = 0; }
							else if (strcmp(token_table[i]->operand[1], "S") == 0) { r2 = 4; }
							else if (strcmp(token_table[i]->operand[1], "T") == 0) { r2 = 5; }
							else if (strcmp(token_table[i]->operand[1], "X") == 0) { r2 = 1; }
							else {}
							//printf("%X%X%X", opcode, r1, r2);
							sprintf(instbuffer, "%02X%X%X", opcode, r1, r2);
							strcat(object_line, instbuffer);
						}
					}
				}
			}
			else // .�� ���� ����
			{

			}
			++i;
		}
		condition_flag = 0;
	}
}
