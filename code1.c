#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

static bool is_vowel_at(const char *s, int i) {
    char c = s[i];
    if (c=='a'||c=='e'||c=='i'||c=='o'||c=='u') return true;
    if (c=='y' && i>0) return !is_vowel_at(s, i-1);
    return false;
}

static bool has_vowel(const char *s, int len) {
    for (int i=0; i<len; i++)
        if (is_vowel_at(s, i)) return true;
    return false;
}

static int measure(const char *s, int len) {
    int m=0;
    bool prev=false;
    for (int i=0; i<len; i++) {
        bool v=is_vowel_at(s,i);
        if (!v && prev) m++;
        prev=v;
    }
    return m;
}

static bool ends_double_cons(const char *s, int len) {
    if (len<2) return false;
    return s[len-1]==s[len-2] && !is_vowel_at(s, len-1);
}

static bool ends_cvc(const char *s, int len) {
    if (len<3) return false;
    return !is_vowel_at(s,len-3) && is_vowel_at(s,len-2) && !is_vowel_at(s,len-1)
           && s[len-1]!='w' && s[len-1]!='x' && s[len-1]!='y';
}

void stem_word(char *out, const char *in) {
    int len = strlen(in);
    for (int i=0; i<=len; i++) out[i] = tolower((unsigned char)in[i]);

    if (len<=2) return;

    if (len>4 && strcmp(out+len-4,"sses")==0) {
        out[len-2]='\0'; len-=2;
    } else if (len>3 && strcmp(out+len-3,"ies")==0) {
        out[len-2]='\0'; len-=2;
    } else if (!(len>2 && strcmp(out+len-2,"ss")==0) && out[len-1]=='s') {
        out[len-1]='\0'; len--;
    }

    bool fixed=false;
    if (len>3 && strcmp(out+len-3,"eed")==0) {
        if (measure(out, len-3)>0) { out[len-1]='\0'; len--; }
    } else if (len>2 && strcmp(out+len-2,"ed")==0) {
        if (has_vowel(out, len-2)) { out[len-2]='\0'; len-=2; fixed=true; }
    } else if (len>3 && strcmp(out+len-3,"ing")==0) {
        if (has_vowel(out, len-3)) { out[len-3]='\0'; len-=3; fixed=true; }
    }

    if (fixed) {
        if (len>=2 && (strcmp(out+len-2,"at")==0 || strcmp(out+len-2,"bl")==0 || strcmp(out+len-2,"iz")==0)) {
            out[len]='e'; out[len+1]='\0'; len++;
        } else if (ends_double_cons(out, len) && out[len-1]!='l' && out[len-1]!='s' && out[len-1]!='z') {
            out[len-1]='\0'; len--;
        } else if (measure(out,len)==1 && ends_cvc(out,len)) {
            out[len]='e'; out[len+1]='\0'; len++;
        }
    }

    if (len>1 && out[len-1]=='y' && has_vowel(out, len-1))
        out[len-1]='i';

    typedef struct { const char *s; const char *r; } SR;
    SR step2[] = {
        {"ational","ate"},{"tional","tion"},{"enci","ence"},{"anci","ance"},
        {"izer","ize"},{"iser","ise"},{"abli","able"},{"alli","al"},
        {"entli","ent"},{"eli","e"},{"ousli","ous"},{"ization","ize"},
        {"isation","ise"},{"ation","ate"},{"ator","ate"},{"alism","al"},
        {"iveness","ive"},{"fulness","ful"},{"ousness","ous"},{"aliti","al"},
        {"iviti","ive"},{"biliti","ble"},{NULL,NULL}
    };
    for (int i=0; step2[i].s; i++) {
        int sl=strlen(step2[i].s);
        if (len>sl && strcmp(out+len-sl,step2[i].s)==0 && measure(out,len-sl)>0) {
            int rl=strlen(step2[i].r);
            strcpy(out+len-sl, step2[i].r);
            len=len-sl+rl; break;
        }
    }

    SR step3[] = {
        {"icate","ic"},{"ative",""},{"alize","al"},{"alise","al"},
        {"iciti","ic"},{"ical","ic"},{"ful",""},{"ness",""},{NULL,NULL}
    };
    for (int i=0; step3[i].s; i++) {
        int sl=strlen(step3[i].s);
        if (len>sl && strcmp(out+len-sl,step3[i].s)==0 && measure(out,len-sl)>0) {
            int rl=strlen(step3[i].r);
            strcpy(out+len-sl, step3[i].r);
            len=len-sl+rl; break;
        }
    }

    const char *step4[]={"ement","ment","ance","ence","ism","ate","iti","ous","ive","ize","ise","ify","ion","al","er","ic",NULL};
    for (int i=0; step4[i]; i++) {
        int sl=strlen(step4[i]);
        if (len>sl && strcmp(out+len-sl,step4[i])==0 && measure(out,len-sl)>1) {
            if (strcmp(step4[i],"ion")==0) {
                char p=out[len-sl-1];
                if (p=='s'||p=='t') { out[len-sl]='\0'; len-=sl; }
            } else { out[len-sl]='\0'; len-=sl; }
            break;
        }
    }

    if (len>1 && out[len-1]=='e') {
        int m=measure(out,len-1);
        if (m>1||(m==1&&!ends_cvc(out,len-1))) { out[len-1]='\0'; len--; }
    }

    if (len>1 && ends_double_cons(out,len) && out[len-1]=='l' && measure(out,len)>1) {
        out[len-1]='\0'; len--;
    }
}

double similarity(const char *a, const char *b) {
    int len_a=strlen(a), len_b=strlen(b);
    int maxLen=len_a>len_b?len_a:len_b;
    if (maxLen==0) return 1.0;
    int matches=0, minLen=len_a<len_b?len_a:len_b;
    for (int i=0; i<minLen; i++)
        if (a[i]==b[i]) matches++;
    return (double)matches/maxLen;
}

bool wordInParagraph(const char *paragraph, const char *searchWord) {
    char para_copy[500];
    strncpy(para_copy, paragraph, sizeof(para_copy)-1);
    para_copy[sizeof(para_copy)-1]='\0';

    char *w=strtok(para_copy, " .,!?;:\"'()-\t\n");
    while (w!=NULL) {
        char lw[100], sw[100];
        stem_word(lw, w);
        stem_word(sw, searchWord);
        if (strcmp(lw,sw)==0 || similarity(lw,sw)>0.7)
            return true;
        w=strtok(NULL, " .,!?;:\"'()-\t\n");
    }
    return false;
}

int main() {
    char **search=NULL, **paragraphs=NULL;
    int *paragraphOfLine=NULL;
    char input[500], line[500];
    int searchCount=0, paraCount=0, paraIndex=1;
    char firstInP[1000][1000];
    bool status=false;
    bool wordfound=false;
    char **syns = NULL;
    int synCount = 0;

    typedef struct {
    char **words;
    int count;
} Group;

Group *groups = NULL;
int groupCount = 0;

FILE *fptr1 = fopen("synonyms.txt", "r");

if (!fptr1) {
    perror("Error opening file");
    return 1;
}

while (fgets(line, sizeof(line), fptr1)) {
    line[strcspn(line, "\n")] = '\0';

    char **wordsTemp = NULL;
    int count = 0;
    char *word = strtok(line, ",");
    while (word != NULL) {

        while (*word == ' ')
            word++;

        wordsTemp = realloc(wordsTemp, (count + 1) * sizeof(char*));
        wordsTemp[count] = malloc(strlen(word) + 1);
        strcpy(wordsTemp[count], word);

        count++;

        word = strtok(NULL, ",");
    }

    groups = realloc(groups, (groupCount + 1) * sizeof(Group));

    groups[groupCount].words = wordsTemp;
    groups[groupCount].count = count;

    groupCount++;
}

fclose(fptr1);


    fgets(input, sizeof(input), stdin);
    input[strcspn(input,"\n")]='\0';


    char *token=strtok(input, " ");
    while (token!=NULL) {
        char stemmed[100];
        stem_word(stemmed, token);
        search=realloc(search,(searchCount+1)*sizeof(char*));
        search[searchCount]=malloc(strlen(stemmed)+1);
        strcpy(search[searchCount], stemmed);
        searchCount++;
        token=strtok(NULL, " ");
    }

    FILE *fptr=fopen("database.txt","r");
    if (!fptr) { perror("Error opening file"); return 1; }

    while (fgets(line, sizeof(line), fptr)) {
        line[strcspn(line,"\n")]='\0';
        if (strlen(line)==0) { paraIndex++; continue; }

        if (paraCount == 0 || paragraphOfLine[paraCount-1] != paraIndex) {
            char copy[500];
            strcpy(copy, line);
            char *first = strtok(copy, " .,!?;:\"'()-\t\n");
            char stemmed_first[100];
            stem_word(stemmed_first, first);
            strcpy(firstInP[paraIndex], stemmed_first);
        }

        paragraphs=realloc(paragraphs,(paraCount+1)*sizeof(char*));
        paragraphs[paraCount]=malloc(strlen(line)+1);
        strcpy(paragraphs[paraCount], line);
        paragraphOfLine=realloc(paragraphOfLine,(paraCount+1)*sizeof(int));
        paragraphOfLine[paraCount]=paraIndex;
        paraCount++;
    }
    fclose(fptr);

    for (int i=0; i<searchCount; i++) {
        for (int j=0; j<paraCount; j++) {
            if (wordInParagraph(paragraphs[j], search[i])) {
                status=true;
                break;
            } else {
                for (int k=0; k<groupCount; k++) {
                    bool searchInGroup = false;
                    for (int g=0; g<groups[k].count; g++) {
                        char sg[100];
                        stem_word(sg, groups[k].words[g]);
                        if (strcmp(search[i], sg) == 0) {
                            searchInGroup = true;
                            break;
                        }
                    }
                    if (searchInGroup) {
                        for (int g=0; g<groups[k].count; g++) {
                            if (wordInParagraph(paragraphs[j], groups[k].words[g])) {
                                status = true;
                                break;
                            }
                        }
                    }
                    if (status) break;
                }
            }
            if (status) break;
        }
        if (status) break;
    }

    if (!status) printf("no match :(\n");

    int N=paraIndex-1;
    double *wVal=malloc(searchCount*sizeof(double));

    for (int i=0; i<searchCount; i++) {
        int wc=0, pc=0, lastPara=-1;
        wordfound = false;

        for (int j=0; j<paraCount; j++) {
            bool directMatch = wordInParagraph(paragraphs[j], search[i]);
            bool synMatch    = false;

            if (directMatch) {
                wc++;
                wordfound = true;
            } else {
                for (int k=0; k<groupCount && !synMatch; k++) {
                    bool searchInGroup = false;
                    for (int g=0; g<groups[k].count; g++) {
                        char sg[100];
                        stem_word(sg, groups[k].words[g]);
                        if (strcmp(search[i], sg) == 0) {
                            searchInGroup = true;
                            break;
                        }
                    }
                    if (searchInGroup) {
                        for (int g=0; g<groups[k].count; g++) {
                            if (wordInParagraph(paragraphs[j], groups[k].words[g])) {
                                synMatch = true;
                                break;
                            }
                        }
                    }
                }
                if (synMatch) {
                    wc++;
                }
            }

            if ((directMatch || synMatch) && paragraphOfLine[j] != lastPara) {
                pc++;
                lastPara = paragraphOfLine[j];
            }
        }

        wVal[i] = (pc!=0) ? wc*log((double)N/pc) : 0;

        if (!wordfound) {
            wVal[i] *= 0.8;
        }
    }

    double *pVal=calloc(paraCount, sizeof(double));
    for (int i=0; i<paraCount; i++) {
        for (int j=0; j<searchCount; j++) {
            if (wordInParagraph(paragraphs[i], search[j])) {
                pVal[i] += wVal[j];
            } else {
                bool synMatch = false;
                for (int k=0; k<groupCount && !synMatch; k++) {
                    bool searchInGroup = false;
                    for (int g=0; g<groups[k].count; g++) {
                        char sg[100];
                        stem_word(sg, groups[k].words[g]);
                        if (strcmp(search[j], sg) == 0) {
                            searchInGroup = true;
                            break;
                        }
                    }
                    if (searchInGroup) {
                        for (int g=0; g<groups[k].count; g++) {
                            if (wordInParagraph(paragraphs[i], groups[k].words[g])) {
                                synMatch = true;
                                break;
                            }
                        }
                    }
                }
                if (synMatch) {
                    pVal[i] += wVal[j] * 0.8;
                }
            }
        }
    }

    if (status) {

    printf("\n");

    double max;
    int max_i;

    max = pVal[0];
    max_i = 0;

    for (int i = 1; i < paraCount; i++)
        if (pVal[i] > max) { max = pVal[i]; max_i = i; }

    printf("best option is: ");
    char output1[100];
    strcpy(output1, firstInP[paragraphOfLine[max_i]]);

    for (int i = 0; output1[i] != '\0'; i++) {
    if (output1[i] == '_')
        printf(" ");
    else
        printf("%c", output1[i]);
}

    pVal[max_i] = -1;


    max = pVal[0];
    max_i = 0;
    printf("\n");

    for (int i = 1; i < paraCount; i++)
        if (pVal[i] > max) { max = pVal[i]; max_i = i; }

    printf("second best option is: ");
    char output2[100];
    strcpy(output2, firstInP[paragraphOfLine[max_i]]);

    for (int i = 0; output2[i] != '\0'; i++) {
    if (output2[i] == '_')
        printf(" ");
    else
        printf("%c", output2[i]);
}

    pVal[max_i] = -1;


    max = pVal[0];
    max_i = 0;
    printf("\n");

    for (int i = 1; i < paraCount; i++)
        if (pVal[i] > max) { max = pVal[i]; max_i = i; }

    printf("third best option is: ");
    char output3[100];
    strcpy(output3, firstInP[paragraphOfLine[max_i]]);

    for (int i = 0; output3[i] != '\0'; i++) {
    if (output3[i] == '_')
        printf(" ");
    else
        printf("%c", output3[i]);
}
    }

    for (int i=0; i<searchCount; i++) free(search[i]);
    free(search);
    for (int i=0; i<paraCount; i++) free(paragraphs[i]);
    free(paragraphs);
    free(paragraphOfLine);
    free(wVal);
    free(pVal);

    return 0;
}