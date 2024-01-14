#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

// Input: Revised Database
// Output: UTree

struct Item {
  string label;
  int value;
  int support;

  Item(string label, int quantity) {
    this->label = label;
    this->value = quantity;
  }
};

struct Transaction {
  int id;
  vector<Item> items;
  int twu;

  Transaction(int id, vector<Item> items) {
    this->id = id;
    this->items = items;
    this->twu = 0;
  }
};

/*
  The label refers to the item’s label,interLink points to the next node of the
  same item, parentLink points to the parent node, and utList is a dictionary
  that stores both a transaction’s ID as keys and item’s utility in each
  transaction as values.
*/
struct UTreeNode {
  string label;
  UTreeNode *parent;
  UTreeNode *interLink; 

  // transaction id and weight of the node with label in the transaction
  unordered_map<int, int> utList;

  unordered_map<string, UTreeNode*> children;
};

unordered_map<string, UTreeNode*> previousPointer;

void insertInTree(const Transaction &transaction, const vector<Item> &itemsList,
                  UTreeNode *node) {

  Item firstItem = itemsList[0];
  cout << transaction.id << " " << firstItem.label << endl;

  // NOTE: children are stored by label
  // Check if the node has a child with a label of firstItem
  if (node->children.count(firstItem.label) != 0) {
    node->children[firstItem.label]->utList[transaction.id] = firstItem.value;
    // cout << transaction.id <<  " " << firstItem.value << endl;

    auto remainItems = vector<Item>(itemsList.begin() + 1, itemsList.end());
    if (remainItems.size() > 0) {
      insertInTree(transaction, remainItems, node->children[firstItem.label]);
    }
  } else {
    UTreeNode *child = new UTreeNode();
    child->label = firstItem.label;
    child->parent = node;
    child->utList[transaction.id] = firstItem.value;

    if(previousPointer.count(firstItem.label) == 0){
      child->interLink = nullptr;
    }else{
      child->interLink = previousPointer[firstItem.label];
    }

    previousPointer[firstItem.label] = child;

    node->children[child->label] = child;

    auto remainItems = vector<Item>(itemsList.begin() + 1, itemsList.end());
    if (remainItems.size() > 0) {
      insertInTree(transaction, remainItems, child);
    }
  }

  cout << "UTList: " << node->label << endl;
  for(auto l: node->utList){
    cout << l.first << " " << l.second << ", ";
  }
  cout << endl;
}

UTreeNode *generateUTree(vector<Transaction> &RD) {
  cout << "Generated UTree" << endl;

  // Initializing the root node
  UTreeNode *root = new UTreeNode();


  for (auto transaction : RD) {
    vector<Item> itemsList = transaction.items;

    // This function will insert the node and update root
    insertInTree(transaction, itemsList, root);
    cout << endl;
  }

  return root;
}

vector<Transaction>
generateRevisedDatabase(int minUtil, const vector<Transaction> &Database,
                        unordered_map<string, int> &externalUtility,
                        unordered_map<string, int> &itemsSupport) {
  // Create a revised database, new value = quantity * profit and sort the items
  // in the database in descending order of their support and also calculate the
  // TWU: sum of new value
  auto RD = vector<Transaction>();
  for (auto transaction : Database) {
    auto revisedTransaction = transaction;
    int twu = 0;

    for (auto &item : revisedTransaction.items) {
      // we are multiplying because we have to calculate the new value
      item.value = item.value * externalUtility[item.label];
      item.support = itemsSupport[item.label];

      twu += item.value;
    }

    revisedTransaction.twu = twu;
    sort(revisedTransaction.items.begin(), revisedTransaction.items.end(),
         [](const Item &a, const Item &b) { return a.support > b.support; });

    RD.push_back(revisedTransaction);
  }

  // Get the total of each and find the one which is min
  unordered_map<string, int> itemsTWU;
  for (auto transaction : RD) {
    for (auto &item : transaction.items) {
      itemsTWU[item.label] += transaction.twu;
    }
  }

  for (auto &transaction : RD) {
    vector<Item> bestItems = {};
    int twu = 0;
    for (auto item : transaction.items) {
      if (itemsTWU[item.label] >= minUtil) {
        twu += item.value;
        bestItems.push_back(item);
      }
    }

    transaction.twu = twu;
    transaction.items = bestItems;
  }

  return RD;
}

void PrintOldDatabase(vector<Transaction> &Database) {
  for (int i = 0; i < Database.size(); i++) {
    cout << (i + 1) << ": ";
    for (auto item : Database[i].items) {
      cout << "(" + item.label + ", " + to_string(item.value) + ")";
    }
    cout << endl;
  }
  cout << endl;
}

void PrintRevisedDatabase(vector<Transaction> &Database) {
  for (int i = 0; i < Database.size(); i++) {
    cout << (i + 1 <= 9 ? " " : "") << (i + 1) << ": ";
    cout << "TWU: " << Database[i].twu << "\t";
    for (auto item : Database[i].items) {
      cout << "(" + item.label + ": " + to_string(item.value) + ") ";
    }
    cout << endl;
  }
  cout << endl;
}

int main() {
  cout << "Tree Generator" << endl;

  // We have to also decide the minimum utility
  int minUtil = 90;

  // We have a database which has transactions, each transaction has list of
  // items with label and quantity associated with it
  vector<Transaction> Database = {
      Transaction(1,{Item("a", 2), Item("b", 6), Item("c", 2), Item("d", 2),
                   Item("e", 6)}),
      Transaction(2,{Item("a", 3), Item("b", 2), Item("c", 3), Item("d", 2)}),
      Transaction(3,{Item("a", 3), Item("c", 2), Item("d", 2), Item("e", 3),
                   Item("g", 3)}),
      Transaction(4,{Item("a", 4), Item("c", 4), Item("d", 4), Item("e", 12),
                   Item("f", 12)}),
      Transaction(5,{Item("a", 3), Item("c", 4), Item("d", 3), Item("f", 4)}),
      Transaction(6,{Item("a", 2), Item("c", 3), Item("d", 2), Item("f", 3)}),
      Transaction(7,{Item("b", 2), Item("c", 4), Item("d", 2)}),
      Transaction(8,{Item("b", 5), Item("c", 3), Item("d", 3), Item("e", 5)}),
      Transaction(9,{Item("d", 3), Item("g", 1)}),
      Transaction(10,{Item("a", 6), Item("c", 5)}),
      Transaction(11,{Item("a", 2), Item("c", 2)}),
  };

  // We should also have the external utility(profit) and the items support
  unordered_map<string, int> externalUtility = {
      {"a", 3}, {"b", 4}, {"c", 4}, {"d", 1}, {"e", 5}, {"f", 4}, {"g", 1},
  };

  unordered_map<string, int> itemsSupport = {
      {"a", 8}, {"b", 4}, {"c", 10}, {"d", 9}, {"e", 4}, {"f", 3}, {"g", 2},
  };

  PrintOldDatabase(Database);

  // Revising the Database
  auto RD =
      generateRevisedDatabase(minUtil, Database, externalUtility, itemsSupport);
  PrintRevisedDatabase(RD);

  /*
    Search Space. The proposed ECoHUPM algorithm
    utilizes a set-enumeration tree as a search space, whose
    efficiency has been verified in pattern mining. Reversed
    depth-first search traversal is adopted as shown in Figure 1
    to facilitate the search tree. Note that the ECoHUPM uses
    the support descending order to revise database and then to
    construct the UTtree. Hence, with reversed depth-first
    search, the mining order for the running example is
    f≺e≺b≺a≺d≺c

    Utility Tree and Correlation Utility-List Structures.
    Once the database is revised, the proposed ECoHUPM algorithm constructs the
    utility tree (UTtree). A UTtree is a concise structure that stores
    sufficient information for facilitating the mining of Correlated High
    Utility Itemsets in a single phase.
  */

  // Generate the UTree
  auto root = generateUTree(RD);

  cout << "Previous List" << endl;
  auto curr = previousPointer["a"];

  while(curr){
    cout << "UTList: " << curr->label << endl;
    for(auto l: curr->utList){
      cout << l.first << " " << l.second << ", ";
    }
    cout << endl;

    curr = curr->interLink;
  }

  return 0;
}
