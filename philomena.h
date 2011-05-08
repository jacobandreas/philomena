
#define LOWNOTE 0
#define HIGHNOTE 48

#define MAX_NOTES 100

#define RULE_LEN 100
#define MAX_RULES 100
#define KEEP_PROB 8

#define RULE_ALLOWS 1
#define RULE_FORBIDS 2
#define RULE_SKIPS 0

#define RELATIVE 1
#define PREVIOUS 2

struct chord {

	int s, a, t, b;

};

int* load_bassline(char* filename, int *blen);

char** load_rules(char* filename, int *rlen);

struct chord* generate_chords(int *bassline, int blen, char **rules, int rlen); 

int allowed(char **rules, int rlen, struct chord *c0, struct chord *c1);

int rule_match(char *rule, struct chord *c0, struct chord *c1);

int rule_match_c(char *rule, struct chord *c1);

int rule_match_m(char *rule1, char *rule2, struct chord *c0, struct chord *c1);

int note_match(char *rule, int note, int rnote);

int note_match_m(char* rule, int note, int rnote, int pnote);

int numeric(char* test);

char* ltonote(int l, char* notename);
