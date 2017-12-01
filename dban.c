#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NUM_CHARS 256
#define MAX_STACK_SIZE 1024

#define DEBUG

typedef struct{
	char has[NUM_CHARS+1];
} AttributeSet;

typedef struct f{
	AttributeSet lhs;
	AttributeSet rhs;
	struct f* next;
} Functional;

typedef struct k{
	AttributeSet attr;
	struct k* next;
} KeyChain;

Functional* first = NULL;
Functional* last = NULL;

int chars_in_relation[NUM_CHARS];
int chars_in_lhs[NUM_CHARS];
int chars_in_rhs[NUM_CHARS];
int input_stack_top = 0;
int line = 1;

char read_stack[MAX_STACK_SIZE];
int read_stack_top = 0;

void throw_error(char* msg){
	printf("Error line %d: %s", line, msg);
	exit(1);
}

void clear_sides(){
	memset(chars_in_lhs, 0, sizeof(int) * NUM_CHARS);
	memset(chars_in_rhs, 0, sizeof(int) * NUM_CHARS);
}

void init(){
	for(int i=0; i<NUM_CHARS; i++) chars_in_relation[i] = 0;

	clear_sides();
}

void eat_whitespace(FILE* fp){
	char rc;
	while(1){
		fscanf(fp, "%c", &rc);
		fseek(fp, -1, SEEK_CUR);

		if(rc != ' ' && rc != '\t') break;
	}
}

Functional* make_new_functional(){
	Functional* res = malloc(sizeof(Functional));
	res->next = NULL;

	int at = 0;
	int atlhs = 0;
	int atrhs = 0;
	while(at < NUM_CHARS){
		if(chars_in_lhs[at]) res->lhs.has[atlhs++] = at;
		if(chars_in_rhs[at]) res->rhs.has[atrhs++] = at;
		at++;
	}

	return res;
}

void add_functional(){
	Functional* n = make_new_functional();

	if(first == NULL){
		first = n;
		last = n;
	}else{
		last->next = n;
		last = n;
	}
}

char token;

void read_char_c(FILE* fp, char c
#ifdef DEBUG
		, int called_from_line
#endif
		){
	token = fgetc(fp);
	if(token != c){
		char msg[30];
		sprintf(msg, "Expected %c but found %c!", c, token);
		throw_error(msg);
	}
#ifdef DEBUG
	printf("Read char %c from line %d\n", token, called_from_line);
#endif
}

char read_char(FILE* fp
#ifdef DEBUG
		, int called_from_line
#endif
		){
	token = fgetc(fp);
#ifdef DEBUG
	printf("Read char %c (%d) from line %d\n", token, (int)token, called_from_line);
#endif
	return token;
}

#ifdef DEBUG
	#define read_char(E) read_char(E, __LINE__)
	#define read_char_c(E,F) read_char_c(E,F, __LINE__)
#endif

void add_symbol(char s){
	chars_in_relation[(int)s] = 1;
}

/*
 * P ::= H T
 * H ::= (some character not newline, -, > or whitespace) C
 * C ::= (some character not newline, -, > or whitespace) C
 * C ::=
 * T ::= T' T
 * T ::=
 * T' ::= C "->" C newline
 */


void P(FILE* fp);
int C(FILE* fp);
int Tm(FILE* fp);
void T(FILE* fp);
int H(FILE* fp);
void P(FILE* fp);


void P(FILE* fp){
	H(fp);
	T(fp);
}

int H(FILE* fp){
	read_char(fp);
	if(token == '-' || token == '>') throw_error("Characters '-' and '>' cannot be attribute names.");
	add_symbol(token);
	int read_file = C(fp)
	for(int i=0; i<read_stack_top; i++) add_symbol(read_stack[i]);
	line++;

	return read_file;
}

int C(FILE* fp){
	char rc;

	read_stack_top = 0;
	while(1){
		read_char(fp);
		if(token == '\n' || token == '-' || token == '>') break;
		if(token == '\r'){
			read_char_c(fp,'\n');
			break;
		}
		if(token == EOF) return 1;

		if(token == ' ' || token == '\t') continue;
		read_stack[read_stack_top++] = token;
	}

	return 0;
}

void T(FILE* fp){
	read_char();
	if(token == )
	while(!Tm(fp)){}
}

int Tm(FILE* fp){
	int at_file_end;
	Functional* n = malloc(sizeof(Functional));

	at_file_end = C(fp);
	for(int i=0; i<read_stack_top; i++) n->lhs.has[read_stack[i]] = 1;
	if(at_file_end){
		if(read_stack_top == 0){
			return 1;
		}else{
			throw_error("A functional must contain a ->");
		}
	}

	read_char_c(fp,'>');

	at_file_end = C(fp);
	for(int i=0; i<read_stack_top; i++) n->rhs.has[read_stack[i]] = 1;

	if(first == NULL) first = n, last = n;
	else last->next = n, last = n;

	return at_file_end;
}

void print_attr_set(AttributeSet* as){
	for(int i=0; i<NUM_CHARS; i++)
		if(as->has[i]) printf("%c",(char)i);
}

void closure(AttributeSet* as){
	while(1){
		Functional* at = first;
		int num_changes = 0;
		while(at != NULL){
			int lhs_satisfied = 1;
			for(int i=0; i<NUM_CHARS; i++)
				lhs_satisfied = lhs_satisfied && (!at->lhs.has[i] || (at->lhs.has[i] && as->has[i]));

			if(lhs_satisfied){
				for(int i=0; i<NUM_CHARS; i++){
					if(as->has[i] != (as->has[i] || at->rhs.has[i])){
						num_changes++;
					}
					as->has[i] = as->has[i] || at->rhs.has[i];
				}
			}

			at = at->next;
		}

		if(num_changes == 0) break;
	}
}

int has_all(AttributeSet* as){
	for(int i=0; i<NUM_CHARS; i++){
		if(chars_in_relation[i] == 1 && as->has[i] != 1){
			return 0;
		}
	}
	return 1;
}

int attribute_set_size(AttributeSet* as){
	int res = 0;
	for(int i=0; i<NUM_CHARS; i++) res += as->has[i];
	return res;
}

KeyChain* key_chain_first = NULL;
KeyChain* key_chain_end = NULL;

KeyChain* final_key_chain_first = NULL;
KeyChain* final_key_chain_end = NULL;

KeyChain** key_map;
int** g;

void find_all_keys(AttributeSet* base, int at, int can_add){
	AttributeSet r;
	AttributeSet* result = &r;
	memcpy(result, base, sizeof(AttributeSet));
	closure(result);

	if(has_all(result) && can_add){
		if(key_chain_first == NULL){
			key_chain_first = malloc(sizeof(KeyChain));
			memcpy(&(key_chain_first->attr), base, sizeof(AttributeSet));
			key_chain_end = key_chain_first;
		}else{
			key_chain_end->next = malloc(sizeof(KeyChain));
			memcpy(&(key_chain_end->next->attr), base, sizeof(AttributeSet));
			key_chain_end = key_chain_end->next;
		}
		can_add = 0;
	}

	while(!chars_in_relation[at] && at < NUM_CHARS) at++;

	if(at == NUM_CHARS) return;

	base->has[at] = 1;
	find_all_keys(base, at+1, 1);

	base->has[at] = 0;
	find_all_keys(base, at+1, can_add);
}

int is_subset_of(AttributeSet* a, AttributeSet* b){
	// Returns 1 if a is a subset of b

	for(int i=0; i<NUM_CHARS; i++) if(a->has[i]) if(!b->has[i]) return 0;
	return 1;
}

AttributeSet key_filter;
void add_to_filter(KeyChain* k){
	for(int i=0; i<NUM_CHARS; i++)
		key_filter.has[i] = key_filter.has[i] || k->attr.has[i];
}

int is_3nf(Functional* f){
	KeyChain* k = final_key_chain_first;
	while(k != NULL){
		if(is_subset_of(&k->attr,&f->lhs)) return 1;
		k = k->next;
	}

	return is_subset_of(&f->rhs, &key_filter);
}

int is_bcnf(Functional* f){
	KeyChain* k = final_key_chain_first;
	while(k != NULL){
		if(is_subset_of(&k->attr, &f->lhs)) return 1;
		k = k->next;
	}

	return 0;
}

int print_superkeys = 0;
int print_keys = 0;
int print_3nf = 0;
int print_bcnf = 0;

int set_option(char* flag){
	if(flag[0] != '-'){
		printf("Error: Unknown option %s\n", flag);
		return 0;
	}

	if(strcmp(flag, "-superkeys") == 0) print_superkeys = 1;
	if(strcmp(flag, "-keys") == 0) print_keys = 1;
	if(strcmp(flag, "-3nf") == 0) print_3nf = 1;
	if(strcmp(flag, "-bcnf") == 0) print_bcnf = 1;

	if(strcmp(flag, "-h") == 0 || strcmp(flag, "-help") == 0){
		printf("Usage: closure <filename> [options]\n");
		printf("Options:\n");
		printf("	-keys: Show all keys.\n");
		printf("	-superkeys: Show all superkeys.\n");
		printf("	-bcnf: Is the relation in bcnf?\n");
		printf("	-3nf: Is the relation in 3nf?\n");
		return 0;
	}

	return 1;
}

int main(int argc, char** argv){
	if(argc < 2){
		printf("Error: No input files! (fatal)\n");
		exit(0);
	}
	if(strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"-help") == 0){
		set_option(argv[1]);
		return 1;
	}

	int at_arg = 2;
	while(at_arg < argc){
		if(!set_option(argv[at_arg++])) return 1;
	}


	FILE* input_file = fopen(argv[1], "r");

	init();

	P(input_file);

	int num_attrs = 0;
	printf("The relation contains attributes: \n");
	for(int i=0; i<NUM_CHARS; i++){
		if(chars_in_relation[i]) printf("%c", (char)i), num_attrs++;
	}
	printf("\n");
	printf("\n");
	
	printf("The relation contains functionals: \n");
	Functional* at = first;
	int functnum = 0;
	while(at != NULL){
		printf("%d: ", ++functnum);
		for(int i=0; i<NUM_CHARS; i++) if(at->lhs.has[i]) printf("%c", (char)i);
		printf(" -> ");
		for(int i=0; i<NUM_CHARS; i++) if(at->rhs.has[i]) printf("%c", (char)i);
		printf("\n");

		at = at->next;
	}

	AttributeSet as1;
	memset(as1.has, 0, sizeof(as1.has));
	find_all_keys(&as1, 0, 1);

	printf("\n");
	print_superkeys ? printf("Super keys:\n") : 0;
	KeyChain* at_key = key_chain_first;
	int keynum = 0;
	while(at_key != NULL){
		keynum++;
		for(int i=0; i<NUM_CHARS; i++) if(at_key->attr.has[i]) print_superkeys ? printf("%c", (char)i) : 0;

		at_key = at_key->next;
		print_superkeys ? printf("\n") : 0;
	}

	int number_of_keys = keynum;
	key_map = malloc(number_of_keys * sizeof(KeyChain*));
	g = malloc(number_of_keys * sizeof(int*));
	g[0] = malloc(number_of_keys * number_of_keys * sizeof(int));
	for(int i=0; i<number_of_keys; i++){
		g[i] = g[0] + i*number_of_keys;
	}
	memset(g[0], 0, number_of_keys * number_of_keys * sizeof(int));

	at_key = key_chain_first;
	keynum = 0;
	while(at_key != NULL){
		key_map[keynum++] = at_key;
		at_key = at_key->next;
	}

	for(int i=0; i<number_of_keys; i++){
		for(int j=0; j<number_of_keys; j++){
			if(i == j) continue;
			if(is_subset_of(&key_map[i]->attr, &key_map[j]->attr)) g[j][i] = 1;
		}
	}


	for(int i=0; i<number_of_keys; i++){
		int found_cex = 0;
		for(int j=0; j<number_of_keys; j++){
			if(g[i][j] != 0){
				found_cex = 1;
				break;
			}
		}
		if(found_cex == 0) {
			if(final_key_chain_first == NULL){
				KeyChain* n = malloc(sizeof(KeyChain));
				memcpy(n, key_map[i], sizeof(KeyChain));
				n->next = NULL;
				final_key_chain_first = n;
				final_key_chain_end = final_key_chain_first;
			}else{
				KeyChain* n = malloc(sizeof(KeyChain));
				memcpy(n, key_map[i], sizeof(KeyChain));
				n->next = NULL;
				final_key_chain_end->next = n;
				final_key_chain_end = final_key_chain_end->next;
			}
		}
	}

	memset(key_filter.has, 0, NUM_CHARS);
	printf("\n");
	print_keys ? printf("Keys:\n") : 0;
	at_key = final_key_chain_first;
	while(at_key != NULL){
		add_to_filter(at_key);

		print_keys ? print_attr_set(&at_key->attr) : 0;
		print_keys ? printf("\n") : 0;
		at_key = at_key->next;;
	}


	print_keys ? printf("\n") : 0;

	at = first;
	int relation_3nf = 1;
	int relation_bcnf = 1;
	while(at != NULL){
		if(is_3nf(at) == 0){
			print_3nf ? printf("The functional ") : 0;
			print_3nf ? print_attr_set(&at->lhs) : 0;
			print_3nf ? printf(" -> ") : 0;
			print_3nf ? print_attr_set(&at->rhs) : 0;
			print_3nf ? printf(" breaks the 3NF condition!\n") : 0;
			relation_3nf = 0;
		}
		if(is_bcnf(at) == 0){
			print_bcnf ? printf("The functional ") : 0;
			print_bcnf ? print_attr_set(&at->lhs) : 0;
			print_bcnf ? printf(" -> ") : 0;
			print_bcnf ? print_attr_set(&at->rhs) : 0;
			print_bcnf ? printf(" breaks the BCNF condition!\n") : 0;
			relation_bcnf = 0;
		}
		at = at->next;
	}

	printf("\n");

	if(print_3nf){
		if(relation_3nf) printf("The relation is 3NF\n");
		else printf("The relation is not 3NF\n");
	}
	if(print_bcnf){
		if(relation_bcnf) printf("The relation is BCNF\n");
		else printf("The relation is not BCNF\n");
	}
}

