from RevisedDB import Item, RevisedDB, Transaction


class UTreeNode:
    def __init__(self, label: str, parent: 'UTreeNode | None', interLink: 'UTreeNode | None') -> None:
        self.label: str = label
        self.parent = parent
        self.interLink = interLink

        self.utList: dict[int, int] = {}
        self.children: dict[str, 'UTreeNode'] = {}


class UTree:
    previous_pointer: dict[str, UTreeNode] = {}
    root: UTreeNode | None = None

    def __init__(self, revised_db: RevisedDB) -> None:
        self.generateUTree(revised_db)

    def generateUTree(self, revised_db: RevisedDB):
        self.root = UTreeNode("*", None, None)

        for transaction in revised_db.transactions:
            items_list = transaction.items

            self.insert_transaction(transaction, items_list, self.root)

    def insert_transaction(self, transaction: Transaction, items_list: list[Item], node: UTreeNode):
        if len(items_list) == 0:
            return

        first_item = items_list[0]

        if (first_item.label in node.children):
            node.children[first_item.label].utList[transaction.id] = first_item.value

            remain_items = items_list[1:]
            if len(remain_items) > 0:
                self.insert_transaction(
                    transaction, remain_items, node.children[first_item.label])
        else:
            inter_link = None
            if first_item.label in self.previous_pointer:
                inter_link = self.previous_pointer[first_item.label]

            child = UTreeNode(first_item.label, node, inter_link)
            self.previous_pointer[first_item.label] = child
            child.utList[transaction.id] = first_item.value

            node.children[child.label] = child

            remain_items = items_list[1:]
            if len(remain_items) > 0:
                self.insert_transaction(transaction, remain_items, child)
