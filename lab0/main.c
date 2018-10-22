#include <stdio.h>
#include <string.h>

#include "util.h"
#include "slp.h"
#include "prog1.h"

typedef struct table *Table_;
struct table {string id; int value; Table_ tail;};
typedef struct intAndTable *IntAndTable_;
struct intAndTable {int i; Table_ t;};


int maxargs(A_stm);
int maxargs_exp(A_exp);
int maxargs_expList(A_expList);
int max(int, int);

Table_ Table(string, int, struct table *);
IntAndTable_ IntAndTable(int, Table_);
Table_ interpStm(A_stm s, Table_ t);
IntAndTable_ interpExp(A_exp e, Table_ t);
int lookup(Table_ t, string key);
Table_ update(Table_ t, string key, int value);
void interp(A_stm prog);


int maxargs(A_stm prog){
	switch(prog->kind) {
		case A_compoundStm: return max(maxargs(prog->u.compound.stm1), maxargs(prog->u.compound.stm2));
		case A_assignStm: return maxargs_exp(prog->u.assign.exp);
		case A_printStm: return maxargs_expList(prog->u.print.exps);
	}
	return 0;
}

int maxargs_exp(A_exp exp) {
	switch(exp->kind) {
		case A_idExp: case A_numExp: return 0;
		case A_opExp: return max(maxargs_exp(exp->u.op.left), maxargs_exp(exp->u.op.right));
		case A_eseqExp: return max(maxargs(exp->u.eseq.stm), maxargs_exp(exp->u.eseq.exp));
	}
	return 0;
}

int maxargs_expList(A_expList exp_l) {
	switch(exp_l->kind) {
		case A_pairExpList: return max(maxargs_exp(exp_l->u.pair.head), 1 + maxargs_expList(exp_l->u.pair.tail));
		case A_lastExpList: return max(maxargs_exp(exp_l->u.last), 1);
	}
	return 0;
}

int max(int a, int b) {
	return (a > b)? a: b;
}

Table_ Table(string id, int value, struct table *tail) {
	Table_ t = malloc(sizeof(*t));
	t->id = id; 
	t->value = value; 
	t->tail = tail;
	return t;
}

IntAndTable_ IntAndTable(int i, Table_ t) {
	IntAndTable_ it = malloc(sizeof(*it));
	it->i = i;
	it->t = t;
	return it;
}

Table_ interpStm(A_stm s, Table_ t) {
	switch(s->kind) {
		case A_compoundStm: {
			return interpStm(s->u.compound.stm2, interpStm(s->u.compound.stm1, t));
		}
		case A_assignStm: { 
			IntAndTable_ it = interpExp(s->u.assign.exp, t);
			return update(it->t, s->u.assign.id, it->i);
		}
		case A_printStm: {
			A_expList expl = s->u.print.exps;
			Table_ cur_table = t;
			while (expl->kind != A_lastExpList) {
				IntAndTable_ it = interpExp(expl->u.pair.head, cur_table);
				printf("%d ", it->i);
				cur_table = it->t;
				expl = expl->u.pair.tail;
			}
			// last expl
			IntAndTable_ it = interpExp(expl->u.last, cur_table);
			printf("%d\n", it->i);
			cur_table = it->t;
			return cur_table;
		}
	}
	return NULL;
}

IntAndTable_ interpExp(A_exp e, Table_ t) {
	switch(e->kind) {
		case A_idExp: {
			return IntAndTable(lookup(t, e->u.id), t);
		}
		case A_numExp: {
			return IntAndTable(e->u.num, t);
		}
		case A_opExp: {
			// assume all ops are left-associative
			IntAndTable_ leftIt = interpExp(e->u.op.left, t);
			IntAndTable_ rightIt = interpExp(e->u.op.right, leftIt->t);
			int res = 0;
			switch(e->u.op.oper) {
				case A_plus: res = leftIt->i + rightIt->i; break;
				case A_minus: res = leftIt->i - rightIt->i; break;
				case A_times: res = leftIt->i * rightIt->i; break;
				case A_div: res = leftIt->i / rightIt->i; break;
			}
			return IntAndTable(res, rightIt->t);
		}
		case A_eseqExp: {
			return interpExp(e->u.eseq.exp, interpStm(e->u.eseq.stm, t));
		}
	}
	return NULL;
}

int lookup(Table_ t, string key) {
	while (t != NULL) {
		if (strcmp(t->id, key) == 0) {
			return t->value;
		}
		t = t->tail;
	}
	printf("[Error] Identifier %s does not exist!\n", key);
	return NULL;
}

Table_ update(Table_ t, string key, int value) {
	return Table(key, value, t);
} 

void interp(A_stm prog) {
	interpStm(prog, NULL);
}





// DO NOT CHANGE!
int main(){
	printf(">> Right Prog Section:\n");
	A_stm rp = right_prog();
	printf("the maximum number of arguments of any print statement is %d\n",maxargs(rp));
	interp(rp);

	printf(">> Error Prog Section:\n");
	A_stm ep = error_prog();
	printf("the maximum number of arguments of any print statement is %d\n",maxargs(ep));
	interp(ep);	
	return 0;
}

