from UTree import UTree, UTreeNode
from RevisedDB import RevisedDB
import copy
import time


# Global variable to store final result
SEPERATOR = " <<##>> "
CoHUPs: set[str] = set()


class CoUTListItem:
    def __init__(self) -> None:
        self.nodeNumber: float = -1
        self.nodeSupport: float = -1
        self.nodeUtility: float = -1
        self.prefixPath: dict[str, float] = {}


def get_prefix_path(node: UTreeNode, transacton_found: set[float]) -> dict[str, float]:
    path: dict[str, float] = {}

    currPointer = node.parent
    while currPointer != None and currPointer.parent != None:
        label = currPointer.label

        weight = 0
        for t in transacton_found:
            if t in currPointer.utList:
                weight += currPointer.utList[t]

        path[label] = weight
        currPointer = currPointer.parent

    return path


def get_CoUTList(label: str, tree: UTree):
    CoUTList: list[CoUTListItem] = []

    currPointer = tree.previous_pointer[label]
    while currPointer != None:
        transaction_found: set[float] = set()
        curr_node_utility: float = 0

        for key in currPointer.utList:
            transaction_found.add(key)
            curr_node_utility += currPointer.utList[key]

        item = CoUTListItem()
        item.nodeUtility = curr_node_utility
        item.nodeSupport = len(currPointer.utList)
        item.prefixPath = get_prefix_path(currPointer, transaction_found)

        CoUTList.append(item)

        currPointer = currPointer.interLink

    CoUTList.reverse()
    for i in range(len(CoUTList)):
        CoUTList[i].nodeNumber = i + 1

    return CoUTList


def get_itemset_kulc(itemSet: str, total_transaction: float, tree: UTree):
    labels = itemSet.split(SEPERATOR)

    count: dict[float, float] = {}
    supports: list[float] = []

    for label in labels:
        support: float = 0
        curr = tree.previous_pointer[label]

        while curr != None:
            support += len(curr.utList)

            for transaction_id in curr.utList:
                count[transaction_id] = count.get(transaction_id, 0) + 1

            curr = curr.interLink

        support = support / total_transaction
        supports.append(support)

    supportOfLabels: float = 0
    for transaction_id in count:
        if count[transaction_id] == len(labels):
            supportOfLabels += 1

    supportOfLabels = supportOfLabels / total_transaction

    kulc: float = 0
    for support in supports:
        kulc += (supportOfLabels / support)
    kulc /= len(labels)

    return kulc


def get_CoUTList_of_itemset(label: str, previousCoUTList: list[CoUTListItem]) -> list[CoUTListItem]:
    coUTList: list[CoUTListItem] = []

    for row in previousCoUTList:
        if label in row.prefixPath:
            newRow = copy.deepcopy(row)
            newRow.nodeUtility += newRow.prefixPath[label]
            newRow.prefixPath.pop(label)
            coUTList.append(newRow)

    return coUTList


def search(X: str, CoUTList: list[CoUTListItem], min_util: float, min_corr: float, total_transactions: float, tree: UTree):
    HUprefixList: set[str] = set()
    prefixList:  set[str] = set()

    for xNode in CoUTList:
        for key_label in xNode.prefixPath:
            if xNode.nodeUtility >= min_util:
                HUprefixList.add(key_label)

            if key_label not in prefixList:
                prefixList.add(key_label)

    for pi in prefixList:
        itemSet: str = pi + SEPERATOR + X

        CoUTListOfItemSet = get_CoUTList_of_itemset(pi, CoUTList)
        kulcItemSet = get_itemset_kulc(itemSet, total_transactions, tree)

        if kulcItemSet >= min_corr:
            if pi in HUprefixList:
                CoHUPs.add(itemSet)
                search(itemSet, CoUTListOfItemSet, min_util,
                       min_corr, total_transactions, tree)
            else:
                uItemSet = 0
                puItemSet = 0

                for co in CoUTListOfItemSet:
                    uItemSet += co.nodeUtility
                    for key_label in co.prefixPath:
                        puItemSet += co.prefixPath[key_label]

                if uItemSet >= min_util:
                    CoHUPs.add(itemSet)

                if uItemSet + puItemSet >= min_util:
                    search(itemSet, CoUTListOfItemSet, min_util,
                           min_corr, total_transactions, tree)


def ECoHUP(transactions_file: str, utils_file: str, min_util: float, min_corr: float):
    revised_db = RevisedDB(transactions_file, utils_file, min_util, min_corr)
    total_transactions = len(revised_db.item_list)

    utree = UTree(revised_db)

    for X in revised_db.item_list[::-1]:
        CoUTList = get_CoUTList(X, utree)

        ux = 0
        pux = 0
        for co in CoUTList:
            ux += co.nodeUtility
            for key_label in co.prefixPath:
                pux += co.prefixPath[key_label]

        if ux >= min_util:
            CoHUPs.add(X)

        if ux + pux >= min_util:
            search(X, CoUTList, min_util, min_corr, total_transactions, utree)

    return CoHUPs


def getPatterns(transactions_file: str, utils_file: str, min_util: float, min_corr: float):
    # clear the global variable
    global CoHUPs
    CoHUPs = set()

    patterns = ECoHUP(transactions_file, utils_file, min_util, min_corr)

    # Result
    result = set()
    for co in patterns:
        lis = co.split(SEPERATOR)
        lis.sort()
        itemSet = " ".join(lis)
        result.add(itemSet)

    result = list(result)
    return result
