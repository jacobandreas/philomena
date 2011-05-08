#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "philomena.h"

// philomena implementation

int main(int argc, char *argv[]) {

	srand(time(0));

	if(argc != 3) {

		printf("Usage: philomena <bassline file> <rule file>\n");
		exit(1);

	}

	int *bassline;
	char **rules;
	struct chord *chords = 0;

	int blen, rlen;
	
	bassline = load_bassline(argv[1], &blen);
	rules = load_rules(argv[2], &rlen);
	
	while(!chords) chords = generate_chords(bassline, blen, rules, rlen); 

	char n1[20];
	char n2[20];
	char n3[20];
	char n4[20];

	printf("melody = \\relative c {\n\n   \\clef treble\n   \\key c \\major\n   \\time 4/4\n\n   ");

	int i;
	for(i = 0; i < blen; i++) {
		printf("<%s %s %s %s> ", ltonote(chords[i].b, n1), ltonote(chords[i].t, n2), ltonote(chords[i].a, n3), ltonote(chords[i].s, n4));
	}

	printf("\n}\n\n\\score {\n   \\new Staff \\melody\n   \\midi { }\n\n}\n");

	return 0;
}

int* load_bassline(char* filename, int* blen) {

	FILE *file = fopen(filename, "r");

	int len = 0, d;

	int *bassptr = malloc(MAX_NOTES * sizeof(int));

	while(fscanf(file, "%u", &d) != EOF) {

		*bassptr = d;
		bassptr++;
		len++;

	}

	*blen = len;

	return bassptr - len;

}

char** load_rules(char* filename, int* rlen) {

	int len = 0;

	char** ruleptr = malloc(MAX_RULES * sizeof(char*));

	FILE* file = fopen(filename, "r");

	char* thisrule = malloc(RULE_LEN * sizeof(char));
	
	while(fgets(thisrule, RULE_LEN, file)) {

		*ruleptr = thisrule;
		*strchr(thisrule, '\n') = '\0';
		ruleptr++;
		len++;
		thisrule = malloc(RULE_LEN*sizeof(char));

	}

	*rlen = len;

	return ruleptr - len;

}

struct chord* generate_chords(int *bassline, int blen, char **rules, int rlen) {

	struct chord *chords = malloc(MAX_NOTES * sizeof(struct chord));

	int s, a, t, bi;

	for(bi = 0; bi < blen; bi++) {

		int b = bassline[bi];
			
		struct chord *c0 = (bi == 0 ? 0 : chords - 1);

		for(t = LOWNOTE; t < HIGHNOTE; t++) {
		for(a = LOWNOTE; a < HIGHNOTE; a++) {
		for(s = LOWNOTE; s < HIGHNOTE; s++) {

			chords->s = s;
			chords->a = a;
			chords->t = t;
			chords->b = b;

			if(allowed(rules, rlen, c0, chords) && (rand() % 10) < KEEP_PROB) {
				chords++;
				goto nextnote; //forgive me father for I have sinned
			}

		}}}

		//fprintf(stderr, "Failed. Trying again...\n");
		return 0;

	nextnote:;
	}

	return chords - blen;

}

int allowed(char **rules, int rlen, struct chord *c0, struct chord *c1) {

	int i,matches = 0, result;

	for(i = 0; i < rlen; i++) {

		result = rule_match(rules[i], c0, c1); 

		if(result == RULE_FORBIDS) matches = 0;

		if(result == RULE_ALLOWS) matches = 1;

	}

	return matches; 

}

int rule_match(char *rule, struct chord *c0, struct chord *c1) {

	if(rule[0] == '\0') return RULE_SKIPS;

	else if(rule[0] == '*')

		return (rule_match_c(rule+1, c1) ? RULE_ALLOWS : RULE_SKIPS);
		
	else if(rule[0] == '!')

		return rule_match_c(rule+1, c1) ? RULE_FORBIDS : RULE_SKIPS;

	else if(rule[0] == '@')
	
		return (rule_match_m(rule+1, strstr(rule, ":") + 1, c0, c1)) ? RULE_ALLOWS : RULE_SKIPS;

	else if(rule[0] == '~')

		return (rule_match_m(rule+1, strstr(rule, ":") + 1, c0, c1)) ? RULE_FORBIDS : RULE_SKIPS;

	else {

		printf("Unrecognized rule classifier: '%c'\n", rule[1]);
		exit(1);

	}

}

int rule_match_c(char *rule, struct chord *c1) {

	//fprintf(stderr, "matching %s against %d %d %d %d\n", rule, c1->b, c1->t, c1->a, c1->s);

	char ss[RULE_LEN], as[RULE_LEN], ts[RULE_LEN], bs[RULE_LEN];

	sscanf(rule, " %s %s %s %s", bs, ts, as, ss);

	// match fixed note
	
	if(numeric(bs) && strtol(bs, 0, 10) != c1->b) {
		return 0; }

	if(numeric(ts) && strtol(ts, 0, 10) != c1->t) {
		return 0; }

	if(numeric(as) && strtol(as, 0, 10) != c1->a) {
		return 0; }

	if(numeric(ss) && strtol(ss, 0, 10) != c1->s) {
		return 0; }


	// match relative note
	
	int* rnote = &(c1->b);
	
	if(bs[0] == '^') {
	
		if(note_match(bs, c1->b, 0)) rnote = &(c1->b);
		else {return 0;}
		
	} else if(ts[0] == '^') {

		if(note_match(ts, c1->t, 0)) rnote = &(c1->t);
		else return 0;

	} else if(as[0] == '^') {

		if(note_match(as, c1->a, 0)) rnote = &(c1->a);
		else return 0;

	} else if(ss[0] == '^') {

		if(note_match(ss, c1->s, 0)) rnote = &(c1->s);
		else return 0;

	}
	
	if(rnote != &(c1->b) && !note_match(bs, c1->b, *rnote)) return 0;
	if(rnote != &(c1->t) && !note_match(ts, c1->t, *rnote)) return 0;
	if(rnote != &(c1->a) && !note_match(as, c1->a, *rnote)) return 0;
	if(rnote != &(c1->s) && !note_match(ss, c1->s, *rnote)) return 0;

	return 1;

}

int rule_match_m(char *rule1, char *rule2, struct chord *c0, struct chord *c1) {

	if(!c0) return 0;
	if(!rule_match_c(rule1, c0)) return 0;

	char ss[RULE_LEN], as[RULE_LEN], ts[RULE_LEN], bs[RULE_LEN];

	sscanf(rule2, " %s %s %s %s", bs, ts, as, ss);

	// match fixed note
	
	if(numeric(bs) && strtol(bs, 0, 10) != c1->b)
		return 0;

	if(numeric(ts) && strtol(ts, 0, 10) != c1->t)
		return 0;

	if(numeric(as) && strtol(as, 0, 10) != c1->a)
		return 0;

	if(numeric(ss) && strtol(ss, 0, 10) != c1->s)
		return 0;

	int bmode, tmode, amode, smode;

	if(strstr(bs, ".") || strstr(bs, "'"))
		bmode = PREVIOUS;
	else bmode = RELATIVE;

	if(strstr(ts, ".") || strstr(ts, "'"))
		tmode = PREVIOUS;
	else tmode = RELATIVE;
		
	if(strstr(as, ".") || strstr(as, "'"))
		amode = PREVIOUS;
	else amode = RELATIVE;

	if(strstr(ss, ".") || strstr(ss, "'"))
		smode = PREVIOUS;
	else smode = RELATIVE;
		

	// match relative or previous note
	
	int* rnote = &(c1->b);
	
	if(bs[0] == '^') {

		if(note_match(bs, c1->b, 0)) rnote = &(c1->b);
		else return 0;
		
	} else if(ts[0] == '^') {

		if(note_match(ts, c1->t, 0)) rnote = &(c1->t);
		else return 0;

	} else if(as[0] == '^') {

		if(note_match(as, c1->a, 0)) rnote = &(c1->a);
		else return 0;

	} else if(ss[0] == '^') {

		if(note_match(ss, c1->s, 0)) rnote = &(c1->s);
		else return 0;

	} 

	if(rnote != &(c1->b) && !note_match(bs, c1->b, bmode == PREVIOUS ? c0->b : *rnote)) return 0;
	if(rnote != &(c1->t) && !note_match(ts, c1->t, tmode == PREVIOUS ? c0->t : *rnote)) return 0;
	if(rnote != &(c1->a) && !note_match(as, c1->a, amode == PREVIOUS ? c0->a : *rnote)) return 0;
	if(rnote != &(c1->s) && !note_match(ss, c1->s, smode == PREVIOUS ? c0->s : *rnote)) return 0;
	
	return 1;

}

int note_match(char *rule, int note, int rnote) {

	//fprintf(stderr, "attempting to match %s with %d; rnote = %d...", rule, note, rnote);

	char* token;
	char qualifier = '=';
	int modifier = 0;
	int success = 0;

	if(*rule == '^')
		rule++;

	if(*rule == '?')
		return 1;
	
	if(*rule == '>' || *rule == '<') {
		qualifier = *rule;
		rule++;
	}

	if(*rule == '+' || *rule == '-' || *rule == '\'' || *rule == '.') {
		if(*rule == '+' || *rule == '\'') modifier = 1;
		else modifier = -1;
		rule++;
	}

	token = strtok(rule, ",");

	while(token) {

		int tnote = strtol(token, 0, 10);

		if(qualifier == '=') {
			if(modifier == 0) {
				if(note == tnote)
					success = 1;
			} else {
				if(rnote + modifier * tnote == note)
					success = 1;
			}
		} else if(qualifier == '>') {
			if(modifier == 0) {
				if(note > tnote)
					success = 1;
			} else {
				if(rnote + modifier * tnote < note)
					success = 1;
			}
		} else {
			if(modifier == 0) {
				if(note < tnote)
					success = 1;
			} else {
				if(rnote + modifier * tnote > note)
					success = 1;
			}
		}

		token = strtok(0, ",");

	}

	//if(success) fprintf(stderr, "%d successfully matched against %d\n", note, rnote);
	//else fprintf(stderr, "%d didn't match against bassline %d with qualifier=%c, modifier=%d\n", note, rnote, qualifier, modifier);
	//
	
	return success;

}

int numeric(char* test) {

	char c;

	while((c=*test++))
		if (!isdigit(c)) return 0;

	return 1;

}

char* ltonote(int l, char* fullname) {

	int base = l % 12;

	int quotes = l / 12;

	switch (base) {

		case 0: strcpy(fullname, "c="); break;
		case 1: strcpy(fullname, "cis="); break;
		case 2: strcpy(fullname, "d="); break;
		case 3: strcpy(fullname, "dis="); break;
		case 4: strcpy(fullname, "e="); break;
		case 5: strcpy(fullname, "f="); break;
		case 6: strcpy(fullname, "fis="); break;
		case 7: strcpy(fullname, "g="); break;
		case 8: strcpy(fullname, "gis="); break;
		case 9: strcpy(fullname, "a="); break;
		case 10: strcpy(fullname, "ais="); break;
		case 11: strcpy(fullname, "b="); break;
	}

	switch (quotes) {

		case 0: strcat(fullname, ""); break;
		case 1: strcat(fullname, "'"); break;
		case 2: strcat(fullname, "''"); break;
		case 3: strcat(fullname, "'''"); break;
		case 4: strcat(fullname, "''''"); break;

	}

	return fullname;

}
