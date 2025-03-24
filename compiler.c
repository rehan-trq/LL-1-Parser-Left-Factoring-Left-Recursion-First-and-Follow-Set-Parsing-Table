#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PRODUCTIONS 100
#define MAX_SYMBOLS 100
#define MAX_RHS 20
#define MAX_SYMBOL_LENGTH 10

// Structure to represent a production
typedef struct {
    char lhs[MAX_SYMBOL_LENGTH];
    char rhs[MAX_RHS][MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH];
    int rhs_count;
    int symbols_in_rhs[MAX_RHS];
} Production;

// Structure to represent the grammar
typedef struct {
    Production productions[MAX_PRODUCTIONS];
    int prod_count;
    char non_terminals[MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
    int non_terminal_count;
    char terminals[MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
    int terminal_count;
    char start_symbol[MAX_SYMBOL_LENGTH];
} Grammar;

// Function to read grammar from file
Grammar read_grammar_from_file(const char* filename);

// Function to perform left factoring
Grammar left_factoring(Grammar g);

// Function to remove left recursion
Grammar remove_left_recursion(Grammar g);

// Function to compute FIRST sets
void compute_first_sets(Grammar g, char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int first_count[MAX_SYMBOLS]);

// Function to compute FOLLOW sets
void compute_follow_sets(Grammar g, char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int first_count[MAX_SYMBOLS], 
                       char follow_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int follow_count[MAX_SYMBOLS]);

// Function to construct LL(1) parsing table
void construct_parsing_table(Grammar g, char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int first_count[MAX_SYMBOLS],
                          char follow_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int follow_count[MAX_SYMBOLS],
                          int parsing_table[MAX_SYMBOLS][MAX_SYMBOLS]);

// Function to print grammar
void print_grammar(Grammar g);

// Function to print parsing table
void print_parsing_table(Grammar g, int parsing_table[MAX_SYMBOLS][MAX_SYMBOLS]);

// Utility functions
int is_non_terminal(char* symbol);
int get_non_terminal_index(Grammar g, char* symbol);
int get_terminal_index(Grammar g, char* symbol);
int contains_epsilon(char set[MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int count);
void add_to_set(char set[MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int* count, char* symbol);

int main() {
    // Read grammar from file
    Grammar g = read_grammar_from_file("D:\\Semester 6\\CC\\A2\\grammer.txt");
    printf("Original Grammar:\n");
    print_grammar(g);
    
    // Perform left factoring
    Grammar g_factored = left_factoring(g);
    printf("\nGrammar after Left Factoring:\n");
    print_grammar(g_factored);
    
    // Remove left recursion
    Grammar g_no_left_recursion = remove_left_recursion(g_factored);
    printf("\nGrammar after Left Recursion Removal:\n");
    print_grammar(g_no_left_recursion);
    
    // Compute FIRST sets
    char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
    int first_count[MAX_SYMBOLS] = {0};
    compute_first_sets(g_no_left_recursion, first_sets, first_count);
    
    // Print FIRST sets
    printf("\nFIRST Sets:\n");
    for (int i = 0; i < g_no_left_recursion.non_terminal_count; i++) {
        printf("FIRST(%s) = { ", g_no_left_recursion.non_terminals[i]);
        for (int j = 0; j < first_count[i]; j++) {
            printf("%s ", first_sets[i][j]);
            if (j < first_count[i] - 1) printf(", ");
        }
        printf("}\n");
    }
    
    // Compute FOLLOW sets
    char follow_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
    int follow_count[MAX_SYMBOLS] = {0};
    compute_follow_sets(g_no_left_recursion, first_sets, first_count, follow_sets, follow_count);
    
    // Print FOLLOW sets
    printf("\nFOLLOW Sets:\n");
    for (int i = 0; i < g_no_left_recursion.non_terminal_count; i++) {
        printf("FOLLOW(%s) = { ", g_no_left_recursion.non_terminals[i]);
        for (int j = 0; j < follow_count[i]; j++) {
            printf("%s ", follow_sets[i][j]);
            if (j < follow_count[i] - 1) printf(", ");
        }
        printf("}\n");
    }
    
    // Construct LL(1) parsing table
    int parsing_table[MAX_SYMBOLS][MAX_SYMBOLS];
    memset(parsing_table, -1, sizeof(parsing_table));
    construct_parsing_table(g_no_left_recursion, first_sets, first_count, follow_sets, follow_count, parsing_table);
    print_parsing_table(g_no_left_recursion, parsing_table);


    // Print parsing table
    // printf("\nLL(1) Parsing Table:\n");
    // // Print table header
    // printf("%10s | ", "");
    // for (int i = 0; i < g_no_left_recursion.terminal_count; i++) {
    //     printf("%10s | ", g_no_left_recursion.terminals[i]);
    // }
    // printf("%10s\n", "$");
    
    // // Print separator
    // for (int i = 0; i < g_no_left_recursion.terminal_count + 2; i++) {
    //     printf("------------");
    // }
    // printf("\n");


    
    // Print table rows
    // for (int i = 0; i < g_no_left_recursion.non_terminal_count; i++) {
    //     printf("%10s | ", g_no_left_recursion.non_terminals[i]);
    //     for (int j = 0; j < g_no_left_recursion.terminal_count + 1; j++) {
    //         if (parsing_table[i][j] != -1) {
    //             printf("%10s | ", g_no_left_recursion.productions[parsing_table[i][j]].lhs);
    //             printf(" -> ");
    //             // Print the production
    //             // ...
    //         } else {
    //             printf("%10s | ", "");
    //         }
    //     }
    //     printf("\n");
    // }
    
    return 0;
}

Grammar read_grammar_from_file(const char* filename) {
    Grammar g;
    g.prod_count = 0;
    g.non_terminal_count = 0;
    g.terminal_count = 0;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file\n");
        exit(1);
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline and skip empty lines
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        
        // Find the "->" arrow in the line
        char *arrow = strstr(line, "->");
        if (!arrow) continue;
        
        // Split into LHS and RHS parts
        *arrow = '\0';
        arrow += 2;  // Skip past "->"
        
        // Trim LHS
        char *lhs = line;
        while (isspace(*lhs)) lhs++;
        char *end = lhs + strlen(lhs) - 1;
        while (end >= lhs && isspace(*end)) {
            *end = '\0';
            end--;
        }
        
        // Set production LHS and update non-terminals list
        strcpy(g.productions[g.prod_count].lhs, lhs);
        int found = 0;
        for (int i = 0; i < g.non_terminal_count; i++) {
            if (strcmp(g.non_terminals[i], lhs) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(g.non_terminals[g.non_terminal_count], lhs);
            g.non_terminal_count++;
        }
        
        // For the first production, set start symbol
        if (g.prod_count == 0) {
            strcpy(g.start_symbol, lhs);
        }
        
        // Process the RHS part manually
        g.productions[g.prod_count].rhs_count = 0;
        char *rhs = arrow;
        // Trim leading whitespace on RHS
        while (*rhs && isspace(*rhs)) rhs++;
        
        // Loop over alternatives separated by '|'
        char *alt = rhs;
        while (alt && *alt) {
            // Look for the next pipe
            char *pipe = strchr(alt, '|');
            if (pipe) {
                *pipe = '\0'; // Terminate current alternative
            }
            
            // Trim alternative
            while (isspace(*alt)) alt++;
            char *alt_end = alt + strlen(alt) - 1;
            while (alt_end >= alt && isspace(*alt_end)) {
                *alt_end = '\0';
                alt_end--;
            }
            
            // Tokenize this alternative by spaces to get individual symbols
            int symbol_index = 0;
            char temp[256];
            strcpy(temp, alt);
            char *token = strtok(temp, " ");
            while (token) {
                // Copy the token into the production alternative
                strcpy(g.productions[g.prod_count].rhs[g.productions[g.prod_count].rhs_count][symbol_index], token);
                // Update non-terminals/terminals lists
                if (isupper(token[0])) {
                    int found = 0;
                    for (int i = 0; i < g.non_terminal_count; i++) {
                        if (strcmp(g.non_terminals[i], token) == 0) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        strcpy(g.non_terminals[g.non_terminal_count], token);
                        g.non_terminal_count++;
                    }
                } else {
                    if (strcmp(token, "epsilon") != 0) {
                        int found = 0;
                        for (int i = 0; i < g.terminal_count; i++) {
                            if (strcmp(g.terminals[i], token) == 0) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            strcpy(g.terminals[g.terminal_count], token);
                            g.terminal_count++;
                        }
                    }
                }
                symbol_index++;
                token = strtok(NULL, " ");
            }
            g.productions[g.prod_count].symbols_in_rhs[g.productions[g.prod_count].rhs_count] = symbol_index;
            g.productions[g.prod_count].rhs_count++;
            
            if (pipe) {
                alt = pipe + 1;
            } else {
                break;
            }
        }
        
        g.prod_count++;
    }
    
    fclose(file);
    return g;
}

int longest_common_prefix_tokens(char alt1[MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH],
    int alt1_len,
    char alt2[MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH],
    int alt2_len) 
    {
        int min_len = (alt1_len < alt2_len) ? alt1_len : alt2_len;
        int i;
        for (i = 0; i < min_len; i++) {
            if (strcmp(alt1[i], alt2[i]) != 0) {
                break;
            }
        }
        return i; // number of matching tokens
    }

/* 
   left_factor_production factors one production p by grouping alternatives 
   that share the same first token. It factors out any group with at least 2 alternatives.
   (This version factors on the first token only.)
*/
void left_factor_production(Grammar *g, Production *p) {
    // We'll create a temporary production to hold the new alternatives.
    Production newProd;
    strcpy(newProd.lhs, p->lhs);
    newProd.rhs_count = 0;
    
    // Create an array to mark which alternatives have been processed.
    int processed[MAX_RHS] = {0};
    
    // For each alternative in p:
    for (int i = 0; i < p->rhs_count; i++) {
        if (processed[i] || p->symbols_in_rhs[i] == 0)
            continue;
        // Group alternatives that share the same first token.
        char firstToken[MAX_SYMBOL_LENGTH];
        strcpy(firstToken, p->rhs[i][0]);
        
        // Start a new group with alternative i.
        int groupCount = 1;
        processed[i] = 1;
        
        // Temporary storage for suffixes in the group.
        // Each suffix is a sequence of tokens from index 1 onward.
        char suffixes[MAX_RHS][MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH];
        int suffixLen[MAX_RHS] = {0};
        
        // Save suffix for alternative i.
        suffixLen[0] = p->symbols_in_rhs[i] - 1;
        for (int k = 1; k < p->symbols_in_rhs[i]; k++) {
            strcpy(suffixes[0][k-1], p->rhs[i][k]);
        }
        
        // Check the remaining alternatives.
        for (int j = i+1; j < p->rhs_count; j++) {
            if (p->symbols_in_rhs[j] == 0) continue;
            if (strcmp(p->rhs[j][0], firstToken) == 0) {
                // Same first token – add to the group.
                processed[j] = 1;
                suffixLen[groupCount] = p->symbols_in_rhs[j] - 1;
                for (int k = 1; k < p->symbols_in_rhs[j]; k++) {
                    strcpy(suffixes[groupCount][k-1], p->rhs[j][k]);
                }
                groupCount++;
            }
        }
        
        if (groupCount >= 2) {
            // Factor this group out.
            // Create a new non-terminal for the factored suffix.
            char new_nt[MAX_SYMBOL_LENGTH];
            sprintf(new_nt, "%s'", p->lhs);
            // Ensure uniqueness by appending additional primes if needed.
            int unique = 0;
            while (!unique) {
                unique = 1;
                for (int x = 0; x < g->non_terminal_count; x++) {
                    if (strcmp(g->non_terminals[x], new_nt) == 0) {
                        strcat(new_nt, "'");
                        unique = 0;
                        break;
                    }
                }
            }
            // Add new_nt to grammar's non-terminals.
            strcpy(g->non_terminals[g->non_terminal_count++], new_nt);
            
            // In the original production, add one alternative: firstToken followed by new_nt.
            strcpy(newProd.rhs[newProd.rhs_count][0], firstToken);
            strcpy(newProd.rhs[newProd.rhs_count][1], new_nt);
            newProd.symbols_in_rhs[newProd.rhs_count] = 2;
            newProd.rhs_count++;
            
            // Create a new production for new_nt.
            Production newProd2;
            strcpy(newProd2.lhs, new_nt);
            newProd2.rhs_count = 0;
            // For each alternative in the group, add its suffix as an alternative.
            for (int gIdx = 0; gIdx < groupCount; gIdx++) {
                int sLen = suffixLen[gIdx];
                int altIndex = newProd2.rhs_count;
                if (sLen == 0) {
                    // If no suffix, add epsilon.
                    strcpy(newProd2.rhs[altIndex][0], "epsilon");
                    newProd2.symbols_in_rhs[altIndex] = 1;
                } else {
                    for (int t = 0; t < sLen; t++) {
                        strcpy(newProd2.rhs[altIndex][t], suffixes[gIdx][t]);
                    }
                    newProd2.symbols_in_rhs[altIndex] = sLen;
                }
                newProd2.rhs_count++;
            }
            // Add newProd2 to grammar.
            g->productions[g->prod_count++] = newProd2;
        } else {
            // Only one alternative had this first token: copy it unchanged.
            // Find the alternative index (which is i).
            for (int t = 0; t < p->symbols_in_rhs[i]; t++) {
                strcpy(newProd.rhs[newProd.rhs_count][t], p->rhs[i][t]);
            }
            newProd.symbols_in_rhs[newProd.rhs_count] = p->symbols_in_rhs[i];
            newProd.rhs_count++;
        }
    }
    
    // Replace p with the new production (which now has factored alternatives).
    *p = newProd;
}

    /*
* The main left_factor function that processes each production by factoring subsets of alternatives.
*/
Grammar left_factoring(Grammar g) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < g.prod_count; i++) {
            int oldCount = g.productions[i].rhs_count;
            left_factor_production(&g, &g.productions[i]);
            if (g.productions[i].rhs_count != oldCount)
                changed = 1;
        }
    }
    return g;

}


// Revised remove_left_recursion that handles productions with no non-left-recursive alternative.
Grammar remove_left_recursion(Grammar g) {
    Grammar result;
    result.prod_count = 0;
    result.non_terminal_count = g.non_terminal_count;
    result.terminal_count = g.terminal_count;
    strcpy(result.start_symbol, g.start_symbol);
    
    // Copy existing non-terminals and terminals.
    for (int i = 0; i < g.non_terminal_count; i++) {
        strcpy(result.non_terminals[i], g.non_terminals[i]);
    }
    for (int i = 0; i < g.terminal_count; i++) {
        strcpy(result.terminals[i], g.terminals[i]);
    }
    
    // Process each production (assumed one production per non-terminal).
    for (int i = 0; i < g.prod_count; i++) {
        Production prod = g.productions[i];
        char A[MAX_SYMBOL_LENGTH];
        strcpy(A, prod.lhs);
        
        // Temporary storage for alternatives:
        // alpha: non-left-recursive alternatives.
        // beta: left-recursive alternatives (with A as the first token).
        char alpha[MAX_RHS][MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH];
        int alpha_count = 0;
        int alpha_symbol_count[MAX_RHS] = {0};
        
        char beta[MAX_RHS][MAX_SYMBOL_LENGTH][MAX_SYMBOL_LENGTH];
        int beta_count = 0;
        int beta_symbol_count[MAX_RHS] = {0};
        
        // Separate alternatives.
        for (int j = 0; j < prod.rhs_count; j++) {
            if (prod.symbols_in_rhs[j] > 0 && strcmp(prod.rhs[j][0], A) == 0) {
                // Left recursive alternative: store its suffix (tokens after A).
                beta_symbol_count[beta_count] = prod.symbols_in_rhs[j] - 1;
                for (int k = 1; k < prod.symbols_in_rhs[j]; k++) {
                    strcpy(beta[beta_count][k - 1], prod.rhs[j][k]);
                }
                beta_count++;
            } else {
                // Non-left-recursive alternative.
                alpha_symbol_count[alpha_count] = prod.symbols_in_rhs[j];
                for (int k = 0; k < prod.symbols_in_rhs[j]; k++) {
                    strcpy(alpha[alpha_count][k], prod.rhs[j][k]);
                }
                alpha_count++;
            }
        }
        
        if (beta_count > 0) {
            // Left recursion exists for A.
            // Generate a new non-terminal name for the left-recursive part.
            char new_nt[MAX_SYMBOL_LENGTH];
            sprintf(new_nt, "%s'", A);
            // Ensure uniqueness: if new_nt is already present, append another prime.
            while(get_non_terminal_index(result, new_nt) != -1) {
                strcat(new_nt, "'");
            }
            // Add new_nt to result's non-terminals.
            strcpy(result.non_terminals[result.non_terminal_count], new_nt);
            result.non_terminal_count++;
            
            // CASE 1: If at least one non-left-recursive alternative exists.
            if (alpha_count > 0) {
                Production newProd;
                strcpy(newProd.lhs, A);
                newProd.rhs_count = 0;
                for (int j = 0; j < alpha_count; j++) {
                    int count = alpha_symbol_count[j];
                    for (int k = 0; k < count; k++) {
                        strcpy(newProd.rhs[newProd.rhs_count][k], alpha[j][k]);
                    }
                    // Append new_nt at the end.
                    strcpy(newProd.rhs[newProd.rhs_count][count], new_nt);
                    newProd.symbols_in_rhs[newProd.rhs_count] = count + 1;
                    newProd.rhs_count++;
                }
                result.productions[result.prod_count] = newProd;
                result.prod_count++;
            } else {
                // CASE 2: No non-left-recursive alternative.
                Production newProd;
                strcpy(newProd.lhs, A);
                newProd.rhs_count = 1;
                int count = beta_symbol_count[0]; // Use the first beta alternative.
                for (int k = 0; k < count; k++) {
                    strcpy(newProd.rhs[0][k], beta[0][k]);
                }
                // Append new_nt.
                strcpy(newProd.rhs[0][count], new_nt);
                newProd.symbols_in_rhs[0] = count + 1;
                result.productions[result.prod_count] = newProd;
                result.prod_count++;
            }
            
            // Create production for the new non-terminal new_nt.
            Production newProd2;
            strcpy(newProd2.lhs, new_nt);
            newProd2.rhs_count = 0;
            for (int j = 0; j < beta_count; j++) {
                int count = beta_symbol_count[j];
                for (int k = 0; k < count; k++) {
                    strcpy(newProd2.rhs[newProd2.rhs_count][k], beta[j][k]);
                }
                // Append new_nt at the end for recursion.
                strcpy(newProd2.rhs[newProd2.rhs_count][count], new_nt);
                newProd2.symbols_in_rhs[newProd2.rhs_count] = count + 1;
                newProd2.rhs_count++;
            }
            // Add an alternative for epsilon.
            strcpy(newProd2.rhs[newProd2.rhs_count][0], "epsilon");
            newProd2.symbols_in_rhs[newProd2.rhs_count] = 1;
            newProd2.rhs_count++;
            
            result.productions[result.prod_count] = newProd2;
            result.prod_count++;
        } else {
            // No left recursion: copy the production as is.
            result.productions[result.prod_count] = prod;
            result.prod_count++;
        }
    }
    
    return result;
}


void compute_first_sets(Grammar g, char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int first_count[MAX_SYMBOLS]) {
    int i, j, k, t;
    // Initialize FIRST sets for all non-terminals to empty.
    for (i = 0; i < g.non_terminal_count; i++) {
        first_count[i] = 0;
    }
    
    // For terminals, we store their FIRST set in the indices after non-terminals (if needed)
    for (i = 0; i < g.terminal_count; i++) {
        strcpy(first_sets[g.non_terminal_count + i][0], g.terminals[i]);
        first_count[g.non_terminal_count + i] = 1;
    }
    
    int changed = 1;
    while(changed) {
        changed = 0;
        // Process each production in the grammar.
        for (i = 0; i < g.prod_count; i++) {
            Production p = g.productions[i];
            // Get the index for the LHS non-terminal.
            int lhs_index = get_non_terminal_index(g, p.lhs);
            if (lhs_index == -1) continue;
            
            // Process each alternative for this production.
            for (j = 0; j < p.rhs_count; j++) {
                // If the alternative is exactly "epsilon", add it.
                if (p.symbols_in_rhs[j] == 1 && strcmp(p.rhs[j][0], "epsilon") == 0) {
                    int exists = 0;
                    for (t = 0; t < first_count[lhs_index]; t++) {
                        if (strcmp(first_sets[lhs_index][t], "epsilon") == 0) {
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists) {
                        strcpy(first_sets[lhs_index][first_count[lhs_index]++], "epsilon");
                        changed = 1;
                    }
                    continue;
                }
                
                // Process the symbols in the alternative left-to-right.
                int allCanBeEpsilon = 1;
                for (k = 0; k < p.symbols_in_rhs[j]; k++) {
                    char *symbol = p.rhs[j][k];
                    // Check if the symbol is terminal or non-terminal by using our grammar.
                    if (get_non_terminal_index(g, symbol) == -1) {
                        // symbol is a terminal; add it and stop.
                        int exists = 0;
                        for (t = 0; t < first_count[lhs_index]; t++) {
                            if (strcmp(first_sets[lhs_index][t], symbol) == 0) {
                                exists = 1;
                                break;
                            }
                        }
                        if (!exists) {
                            strcpy(first_sets[lhs_index][first_count[lhs_index]++], symbol);
                            changed = 1;
                        }
                        allCanBeEpsilon = 0;
                        break; // Stop processing further symbols.
                    } else {
                        // symbol is a non-terminal.
                        int sym_index = get_non_terminal_index(g, symbol);
                        // Add FIRST(symbol) except epsilon to FIRST(lhs)
                        for (t = 0; t < first_count[sym_index]; t++) {
                            if (strcmp(first_sets[sym_index][t], "epsilon") == 0)
                                continue;
                            int exists = 0;
                            for (int u = 0; u < first_count[lhs_index]; u++) {
                                if (strcmp(first_sets[lhs_index][u], first_sets[sym_index][t]) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(first_sets[lhs_index][first_count[lhs_index]++], first_sets[sym_index][t]);
                                changed = 1;
                            }
                        }
                        // Check if FIRST(symbol) contains epsilon.
                        int hasEpsilon = 0;
                        for (t = 0; t < first_count[sym_index]; t++) {
                            if (strcmp(first_sets[sym_index][t], "epsilon") == 0) {
                                hasEpsilon = 1;
                                break;
                            }
                        }
                        if (!hasEpsilon) {
                            allCanBeEpsilon = 0;
                            break;
                        }
                    }
                }
                // If all symbols in the alternative can derive ε, add ε to FIRST(lhs).
                if (allCanBeEpsilon) {
                    int exists = 0;
                    for (t = 0; t < first_count[lhs_index]; t++) {
                        if (strcmp(first_sets[lhs_index][t], "epsilon") == 0) {
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists) {
                        strcpy(first_sets[lhs_index][first_count[lhs_index]++], "epsilon");
                        changed = 1;
                    }
                }
            }
        }
    }
}

void compute_follow_sets(Grammar g, char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], 
                           int first_count[MAX_SYMBOLS],
                           char follow_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH], 
                           int follow_count[MAX_SYMBOLS]) {
    int i, j, k, t, u, v;
    
    // Initialize FOLLOW sets for all non-terminals to empty.
    for (i = 0; i < g.non_terminal_count; i++) {
        follow_count[i] = 0;
    }
    
    // Add '$' to FOLLOW of the start symbol.
    int startIndex = get_non_terminal_index(g, g.start_symbol);
    if (startIndex != -1) {
        strcpy(follow_sets[startIndex][follow_count[startIndex]++], "$");
    }
    
    int changed = 1;
    while (changed) {
        changed = 0;
        // For every production A -> X1 X2 ... Xn.
        for (i = 0; i < g.prod_count; i++) {
            Production p = g.productions[i];
            int A_index = get_non_terminal_index(g, p.lhs);
            if (A_index == -1) continue;
            // For each alternative of the production.
            for (j = 0; j < p.rhs_count; j++) {
                // For each symbol X in the alternative.
                for (k = 0; k < p.symbols_in_rhs[j]; k++) {
                    char *X = p.rhs[j][k];
                    int X_index = get_non_terminal_index(g, X);
                    if (X_index == -1) continue; // X is terminal, so skip.
                    
                    // Process the tail: symbols after X in the alternative.
                    int tail_can_be_epsilon = 1; // Assume tail derives ε until proven otherwise.
                    for (t = k + 1; t < p.symbols_in_rhs[j]; t++) {
                        char *Y = p.rhs[j][t];
                        // Check if Y is terminal or non-terminal.
                        if (get_non_terminal_index(g, Y) == -1) {
                            // Y is terminal; add Y to FOLLOW(X) if not already present.
                            int exists = 0;
                            for (u = 0; u < follow_count[X_index]; u++) {
                                if (strcmp(follow_sets[X_index][u], Y) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(follow_sets[X_index][follow_count[X_index]++], Y);
                                changed = 1;
                            }
                            tail_can_be_epsilon = 0; // Terminal cannot produce ε.
                            break;  // Stop processing further symbols in tail.
                        } else {
                            // Y is non-terminal.
                            int Y_index = get_non_terminal_index(g, Y);
                            // Add FIRST(Y) (excluding ε) to FOLLOW(X).
                            for (u = 0; u < first_count[Y_index]; u++) {
                                if (strcmp(first_sets[Y_index][u], "epsilon") == 0)
                                    continue;
                                int exists = 0;
                                for (v = 0; v < follow_count[X_index]; v++) {
                                    if (strcmp(follow_sets[X_index][v], first_sets[Y_index][u]) == 0) {
                                        exists = 1;
                                        break;
                                    }
                                }
                                if (!exists) {
                                    strcpy(follow_sets[X_index][follow_count[X_index]++], first_sets[Y_index][u]);
                                    changed = 1;
                                }
                            }
                            // Check if FIRST(Y) contains ε.
                            int Y_has_epsilon = 0;
                            for (u = 0; u < first_count[Y_index]; u++) {
                                if (strcmp(first_sets[Y_index][u], "epsilon") == 0) {
                                    Y_has_epsilon = 1;
                                    break;
                                }
                            }
                            if (!Y_has_epsilon) {
                                tail_can_be_epsilon = 0;
                                break; // Stop processing tail.
                            }
                        }
                    } // End processing tail.
                    
                    // If the tail (or no tail) can derive ε, add FOLLOW(A) to FOLLOW(X).
                    if (tail_can_be_epsilon) {
                        for (u = 0; u < follow_count[A_index]; u++) {
                            int exists = 0;
                            for (v = 0; v < follow_count[X_index]; v++) {
                                if (strcmp(follow_sets[X_index][v], follow_sets[A_index][u]) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(follow_sets[X_index][follow_count[X_index]++], follow_sets[A_index][u]);
                                changed = 1;
                            }
                        }
                    }
                } 
            } 
        }
    } 
}


// We encode a table entry as: entry = prodIndex * 1000 + altIndex
// (Assuming prodIndex and altIndex are less than 1000.)

void construct_parsing_table(Grammar g,
                             char first_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH],
                             int first_count[MAX_SYMBOLS],
                             char follow_sets[MAX_SYMBOLS][MAX_SYMBOLS][MAX_SYMBOL_LENGTH],
                             int follow_count[MAX_SYMBOLS],
                             int parsing_table[MAX_SYMBOLS][MAX_SYMBOLS])
{
    // Initialize table cells to -1 (empty).
    for (int i = 0; i < g.non_terminal_count; i++) {
        for (int j = 0; j < g.terminal_count + 1; j++) { // +1 for '$'
            parsing_table[i][j] = -1;
        }
    }

    // Process each production.
    for (int prodIndex = 0; prodIndex < g.prod_count; prodIndex++) {
        int nt_index = get_non_terminal_index(g, g.productions[prodIndex].lhs);
        if (nt_index == -1) continue;

        // For each alternative in this production.
        for (int altIndex = 0; altIndex < g.productions[prodIndex].rhs_count; altIndex++) {
            // If this alternative is exactly epsilon.
            if (g.productions[prodIndex].symbols_in_rhs[altIndex] == 1 &&
                strcmp(g.productions[prodIndex].rhs[altIndex][0], "epsilon") == 0)
            {
                // For each terminal in FOLLOW(LHS), place this production.
                for (int f = 0; f < follow_count[nt_index]; f++) {
                    int col = get_terminal_index(g, follow_sets[nt_index][f]);
                    if (col == -1 && strcmp(follow_sets[nt_index][f], "$") == 0)
                        col = g.terminal_count;
                    if (col == -1)
                        continue;
                    if (parsing_table[nt_index][col] != -1) {
                        printf("Conflict in parsing table at [%s, %s]\n",
                               g.non_terminals[nt_index],
                               (col == g.terminal_count) ? "$" : g.terminals[col]);
                        printf("Grammar is not LL(1)!\n");
                    }
                    // Encode prodIndex and altIndex.
                    parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                }
            }
            else {
                // Compute FIRST for this alternative.
                char first_of_alt[MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
                int count_first = 0;
                int allNullable = 1;  // assume all symbols derive epsilon
                int n = g.productions[prodIndex].symbols_in_rhs[altIndex];
                for (int s = 0; s < n; s++) {
                    char *sym = g.productions[prodIndex].rhs[altIndex][s];
                    int sym_nt_index = get_non_terminal_index(g, sym);
                    if (sym_nt_index == -1) {
                        // terminal: add it and stop.
                        add_to_set(first_of_alt, &count_first, sym);
                        allNullable = 0;
                        break;
                    } else {
                        // non-terminal: add its FIRST (except epsilon).
                        for (int f = 0; f < first_count[sym_nt_index]; f++) {
                            if (strcmp(first_sets[sym_nt_index][f], "epsilon") != 0)
                                add_to_set(first_of_alt, &count_first, first_sets[sym_nt_index][f]);
                        }
                        if (!contains_epsilon(first_sets[sym_nt_index], first_count[sym_nt_index])) {
                            allNullable = 0;
                            break;
                        }
                    }
                }
                if (allNullable)
                    add_to_set(first_of_alt, &count_first, "epsilon");

                // For every terminal in FIRST (except epsilon) fill table.
                int hasEpsilon = 0;
                for (int f = 0; f < count_first; f++) {
                    if (strcmp(first_of_alt[f], "epsilon") == 0) {
                        hasEpsilon = 1;
                        continue;
                    }
                    int col = get_terminal_index(g, first_of_alt[f]);
                    if (col == -1 && strcmp(first_of_alt[f], "$") == 0)
                        col = g.terminal_count;
                    if (col == -1)
                        continue;
                    if (parsing_table[nt_index][col] != -1) {
                        printf("Conflict in parsing table at [%s, %s]\n",
                               g.non_terminals[nt_index],
                               (col == g.terminal_count) ? "$" : g.terminals[col]);
                        printf("Grammar is not LL(1)!\n");
                    }
                    parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                }
                // If epsilon is in FIRST, then for every terminal in FOLLOW(LHS) fill table.
                if (hasEpsilon) {
                    for (int f = 0; f < follow_count[nt_index]; f++) {
                        int col = get_terminal_index(g, follow_sets[nt_index][f]);
                        if (col == -1 && strcmp(follow_sets[nt_index][f], "$") == 0)
                            col = g.terminal_count;
                        if (col == -1)
                            continue;
                        if (parsing_table[nt_index][col] != -1) {
                            printf("Conflict in parsing table at [%s, %s]\n",
                                   g.non_terminals[nt_index],
                                   (col == g.terminal_count) ? "$" : g.terminals[col]);
                            printf("Grammar is not LL(1)!\n");
                        }
                        parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                    }
                }
            }
        }
    }
}



void print_grammar(Grammar g) {
    for (int i = 0; i < g.prod_count; i++) {
        printf("%s -> ", g.productions[i].lhs);
        for (int j = 0; j < g.productions[i].rhs_count; j++) {
            for (int k = 0; k < g.productions[i].symbols_in_rhs[j]; k++) {
                printf("%s ", g.productions[i].rhs[j][k]);
            }
            
            if (j < g.productions[i].rhs_count - 1) {
                printf("| ");
            }
        }
        printf("\n");
    }
}

void print_parsing_table(Grammar g, int parsing_table[MAX_SYMBOLS][MAX_SYMBOLS]) {
    int totalCols = g.terminal_count + 1; // columns for each terminal plus '$'
    // Print header
    printf("%15s", "");
    for (int j = 0; j < g.terminal_count; j++) {
        printf("|%15s", g.terminals[j]);
    }
    printf("|%15s\n", "$");
    for (int j = 0; j < totalCols; j++) {
        printf("+---------------");
    }
    printf("+\n");

    // Print rows for each non-terminal.
    for (int i = 0; i < g.non_terminal_count; i++) {
        printf("%15s", g.non_terminals[i]);
        for (int j = 0; j < totalCols; j++) {
            printf("|");
            if (parsing_table[i][j] != -1) {
                int code = parsing_table[i][j];
                int prodIndex = code / 1000;
                int altIndex = code % 1000;
                Production prod = g.productions[prodIndex];
                char prodStr[256] = "";
                sprintf(prodStr, "%s -> ", prod.lhs);
                // Print the alternative indicated by altIndex.
                for (int k = 0; k < prod.symbols_in_rhs[altIndex]; k++) {
                    strcat(prodStr, prod.rhs[altIndex][k]);
                    if (k < prod.symbols_in_rhs[altIndex] - 1)
                        strcat(prodStr, " ");
                }
                printf("%15s", prodStr);
            } else {
                printf("%15s", "");
            }
        }
        printf("|\n");
        for (int j = 0; j < totalCols; j++) {
            printf("+---------------");
        }
        printf("+\n");
    }
}


int is_non_terminal(char* symbol) {
    return isupper(symbol[0]);
}

int get_non_terminal_index(Grammar g, char* symbol) {
    for (int i = 0; i < g.non_terminal_count; i++) {
        if (strcmp(g.non_terminals[i], symbol) == 0) {
            return i;
        }
    }
    return -1;
}

int get_terminal_index(Grammar g, char* symbol) {
    for (int i = 0; i < g.terminal_count; i++) {
        if (strcmp(g.terminals[i], symbol) == 0) {
            return i;
        }
    }
    return -1;
}

int contains_epsilon(char set[MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(set[i], "epsilon") == 0) {
            return 1;
        }
    }
    return 0;
}

void add_to_set(char set[MAX_SYMBOLS][MAX_SYMBOL_LENGTH], int* count, char* symbol) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(set[i], symbol) == 0) {
            return;
        }
    }
    strcpy(set[*count], symbol);
    (*count)++;
}