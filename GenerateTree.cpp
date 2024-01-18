#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

// Input: Revised Database
// Output: UTree

unordered_set<string> CoHUPs;

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

struct CoUTListItem {
    int nodeNumber, nodeSupport, nodeUtility;
    vector<pair<string, int>> prefixPath;
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

    unordered_map<string, UTreeNode *> children;
};

unordered_map<string, UTreeNode *> previousPointer;

void insertInTree(const Transaction &transaction, const vector<Item> &itemsList,
                  UTreeNode *node) {
    Item firstItem = itemsList[0];
    cout << transaction.id << " " << firstItem.label << endl;

    // NOTE: children are stored by label
    // Check if the node has a child with a label of firstItem
    if (node->children.count(firstItem.label) != 0) {
        node->children[firstItem.label]->utList[transaction.id] =
            firstItem.value;
        // cout << transaction.id <<  " " << firstItem.value << endl;

        auto remainItems = vector<Item>(itemsList.begin() + 1, itemsList.end());
        if (remainItems.size() > 0) {
            insertInTree(transaction, remainItems,
                         node->children[firstItem.label]);
        }
    } else {
        UTreeNode *child = new UTreeNode();
        child->label = firstItem.label;
        child->parent = node;
        child->utList[transaction.id] = firstItem.value;

        if (previousPointer.count(firstItem.label) == 0) {
            child->interLink = nullptr;
        } else {
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
    for (auto l : node->utList) {
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

vector<Transaction> generateRevisedDatabase(
    int minUtil, const vector<Transaction> &Database,
    unordered_map<string, int> &externalUtility,
    unordered_map<string, int> &itemsSupport) {
    // Create a revised database, new value = quantity * profit and sort the
    // items in the database in descending order of their support and also
    // calculate the TWU: sum of new value
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
        sort(
            revisedTransaction.items.begin(), revisedTransaction.items.end(),
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

// ! TODO: Check if better is their
vector<pair<int, string>> getItemList(vector<Transaction> &Database,
                                      unordered_map<string, int> itemsSupport) {
    unordered_set<string> st;
    vector<pair<int, string>> result;

    for (auto t : Database) {
        for (auto item : t.items) {
            if (st.count(item.label) == 0) {
                st.insert(item.label);
                result.push_back({item.support, item.label});
            }
        }
    }

    sort(result.begin(), result.end(),
         [](const pair<int, string> &a, const pair<int, string> &b) {
             return a.first > b.first;
         });
    return result;
}

vector<pair<string, int>> getPrefixPath(UTreeNode *node,
                                        unordered_set<int> &transactionsFound) {
    int maxiCanTake = node->utList.size();
    vector<pair<string, int>> path;

    auto curr = node->parent;
    while (curr->parent != nullptr) {
        string label = curr->label;

        int weight = 0;
        for (auto p : transactionsFound) {
            if (curr->utList.count(p) == 0) continue;
            weight += curr->utList[p];
        }

        path.push_back({label, weight});

        curr = curr->parent;
    }

    return path;
}

vector<CoUTListItem> getCoUTList(string label) {
    vector<CoUTListItem> CoUTList;
    auto curr = previousPointer[label];

    while (curr) {
        // CoUTListItem
        CoUTListItem item;
        item.nodeSupport = curr->utList.size();

        unordered_set<int> transactionsFound;

        int total = 0;
        for (auto p : curr->utList) {
            transactionsFound.insert(p.first);
            total += p.second;
        }
        item.nodeUtility = total;

        // prefix path
        item.prefixPath = getPrefixPath(curr, transactionsFound);

        CoUTList.push_back(item);

        curr = curr->interLink;
    }

    reverse(CoUTList.begin(), CoUTList.end());

    for (int i = 0; i < CoUTList.size(); i++) {
        CoUTList[i].nodeNumber = i + 1;
    }

    return CoUTList;
}

vector<string> split(const string &sentence, const string &separator) {
    string currentToken = "";
    vector<string> tokens;

    int i = 0;
    while (i < sentence.size()) {
        // Check it matches with separator
        bool isValidSeparator = true;
        for (int k = 0; k < separator.size() && i + k < sentence.size(); k++) {
            if (sentence[i + k] != separator[k]) {
                isValidSeparator = false;
                break;
            }
        }

        if (isValidSeparator) {
            i = i + separator.size();
            tokens.push_back(currentToken);
            currentToken.clear();
        } else {
            currentToken.push_back(sentence[i]);
            i++;
        }
    }

    if (currentToken.size() != 0) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

double getItemSetKulc(string itemSet, int totalTransaction) {
    cout << endl;
    cout << endl;
    cout << "Kulc" << endl;

    vector<string> labels = split(itemSet, "<<##>>");
    cout << "labels: ";
    for (auto label : labels) cout << label << " ";
    cout << endl;

    // kulc a, b is made up of support
    // ((support(a, b) / support(a)) + (support(a, b) / support(b))) / 2
    // here 2 is number of items in the itemSet

    // support(a) = count of transaction with a / total transaction
    // support(labels) = total number of transactions in which we have all
    // labels / total transaction

    // map<transaction id, occurrence count>
    unordered_map<int, int> count;
    vector<double> supports;
    for (auto label : labels) {
        double support = 0;
        auto curr = previousPointer[label];
        while (curr) {
            support += curr->utList.size();

            for (auto p : curr->utList) {
                count[p.first]++;
            }

            curr = curr->interLink;
        }

        cout << "count of " << label << " : " << support << endl;
        support = support / totalTransaction;
        cout << "support of " << label << " : " << support << endl;
        supports.push_back(support);
    }

    double supportOfLabels = 0;
    for (auto p : count) {
        if (p.second == labels.size()) {
            supportOfLabels++;
        }
    }

    cout << "count of ab: " << supportOfLabels << endl;
    supportOfLabels = supportOfLabels / totalTransaction;
    cout << "support of ab: " << supportOfLabels << endl;

    // Final kulc calculation
    double kulc = 0;
    for (auto support : supports) {
        kulc += ((double)supportOfLabels / support);
    }

    kulc /= (double)labels.size();
    return kulc;
}

void search(string X, vector<CoUTListItem> &CoUTList, int minUtil, int minCorr,
            int totalTransaction) {
    // TODO: is the below structure correct, it stores label
    unordered_set<string> HUprefixList, prefixList;

    for (auto xNode : CoUTList) {
        for (auto p : xNode.prefixPath) {
            string label = p.first;

            if (xNode.nodeUtility >= minUtil) {
                HUprefixList.insert(label);
            }

            if (HUprefixList.count(label) == 0) {
                prefixList.insert(label);
            }
        }
    }

    for (auto pi : prefixList) {
        string itemSet = pi + "<<##>>" + X;

        // TODO: Scan the CoUTlis(X) to construct the CoUTlist(itemset)
        vector<CoUTListItem> CoUTListOfItemSet;

        double kulcItemSet = getItemSetKulc(itemSet, totalTransaction);
        if (kulcItemSet >= minCorr) {
            if (HUprefixList.count(pi) != 0) {
                CoHUPs.insert(itemSet);
                search(itemSet, CoUTListOfItemSet, minUtil, minCorr,
                       totalTransaction);
            } else {
                // Utility of itemset and pu of item set
                int uItemSet = 0, puItemSet = 0;
                for (auto co : CoUTList) {
                    uItemSet += co.nodeUtility;

                    for (auto p : co.prefixPath) {
                        puItemSet += p.second;
                    }
                }

                if (uItemSet >= minUtil) {
                    CoHUPs.insert(itemSet);
                }

                if (uItemSet + puItemSet >= minUtil) {
                    search(itemSet, CoUTListOfItemSet, minUtil, minCorr,
                           totalTransaction);
                }
            }
        }
    }
}

// TODO: The return type
void ECoHUPM(vector<Transaction> &Database, int minUtil, int minCorr,
             unordered_map<string, int> externalUtility,
             unordered_map<string, int> itemsSupport) {
    PrintOldDatabase(Database);

    // Revising the Database
    auto RD = generateRevisedDatabase(minUtil, Database, externalUtility,
                                      itemsSupport);
    PrintRevisedDatabase(RD);
    int totalTransaction = RD.size();

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
      Once the database is revised, the proposed ECoHUPM algorithm constructs
      the utility tree (UTtree). A UTtree is a concise structure that stores
      sufficient information for facilitating the mining of Correlated High
      Utility Itemsets in a single phase.
    */

    vector<pair<int, string>> itemList = getItemList(RD, itemsSupport);

    cout << "Item List: ";
    for (auto p : itemList) cout << "(" << p.second << ", " << p.first << ") ";
    cout << endl;

    // Generate the UTree
    auto root = generateUTree(RD);

    cout << "Previous List" << endl;
    auto curr = previousPointer["a"];

    while (curr) {
        cout << "UTList: " << curr->label << endl;
        for (auto l : curr->utList) {
            cout << l.first << " " << l.second << ", ";
        }
        cout << endl;

        curr = curr->interLink;
    }

    cout << "############################" << endl;

    for (int i = itemList.size() - 1; i >= 0; i--) {
        string X = itemList[i].second;

        vector<CoUTListItem> CoUTList = getCoUTList(X);

        cout << "Label: " << X << " " << CoUTList.size() << endl;
        // Print the CoUTList
        for (auto co : CoUTList) {
            cout << "Number: " << co.nodeNumber << endl;
            cout << "Support: " << co.nodeSupport << endl;
            cout << "Utility: " << co.nodeUtility << endl;
            cout << "Path: ";
            for (auto p : co.prefixPath)
                cout << "(" << p.first << " " << p.second << ") ";
            cout << endl;
        }

        // 7 to 11 lines in algorithm 2
        int ux = 0, pux = 0;
        for (auto co : CoUTList) {
            ux += co.nodeUtility;

            for (auto p : co.prefixPath) {
                pux += p.second;
            }
        }

        cout << "ux: " << ux << endl;
        cout << "pux: " << pux << endl;

        // if ux + pux is less than correlated utility then x and it's superset
        // will not be COHUIPM
        if (ux >= minUtil) {
            CoHUPs.insert(X);
        }

        if (ux + pux >= minUtil) {
            search(X, CoUTList, minUtil, minCorr, totalTransaction);
        }

        cout << endl;
    }

    cout << "############################" << endl;
}

int main() {
    cout << "Tree Generator" << endl;

    // We have to also decide the minimum utility
    int minUtil = 90;

    // Page 10
    int minCorr = 0.4;

    // We have a database which has transactions, each transaction has list of
    // items with label and quantity associated with it
    vector<Transaction> Database = {
        Transaction(1, {Item("a", 2), Item("b", 6), Item("c", 2), Item("d", 2),
                        Item("e", 6)}),
        Transaction(2,
                    {Item("a", 3), Item("b", 2), Item("c", 3), Item("d", 2)}),
        Transaction(3, {Item("a", 3), Item("c", 2), Item("d", 2), Item("e", 3),
                        Item("g", 3)}),
        Transaction(4, {Item("a", 4), Item("c", 4), Item("d", 4), Item("e", 12),
                        Item("f", 12)}),
        Transaction(5,
                    {Item("a", 3), Item("c", 4), Item("d", 3), Item("f", 4)}),
        Transaction(6,
                    {Item("a", 2), Item("c", 3), Item("d", 2), Item("f", 3)}),
        Transaction(7, {Item("b", 2), Item("c", 4), Item("d", 2)}),
        Transaction(8,
                    {Item("b", 5), Item("c", 3), Item("d", 3), Item("e", 5)}),
        Transaction(9, {Item("d", 3), Item("g", 1)}),
        Transaction(10, {Item("a", 6), Item("c", 5)}),
        Transaction(11, {Item("a", 2), Item("c", 2)}),
    };

    // We should also have the external utility(profit) and the items support
    unordered_map<string, int> externalUtility = {
        {"a", 3}, {"b", 4}, {"c", 4}, {"d", 1}, {"e", 5}, {"f", 4}, {"g", 1},
    };

    unordered_map<string, int> itemsSupport = {
        {"a", 8}, {"b", 4}, {"c", 10}, {"d", 9}, {"e", 4}, {"f", 3}, {"g", 2},
    };

    ECoHUPM(Database, minUtil, minCorr, externalUtility, itemsSupport);

    return 0;
}
