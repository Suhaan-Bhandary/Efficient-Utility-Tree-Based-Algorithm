class Item:
    def __init__(self, label: str, value: float) -> None:
        self.label = label
        self.value = value


class Transaction:
    def __init__(self, id: float, twu: float, items: list[Item]) -> None:
        self.id = id
        self.items = items
        self.twu = twu


class RevisedDB:
    def __init__(self, csv_transactions: str, csv_utils: str, min_util: float, min_corr: float) -> None:
        self.min_util = min_util
        self.min_corr = min_corr

        self.min_util = float('inf')
        self.max_util = float('-inf')

        self.transactions: list[Transaction] = []
        self.utils: dict[str, float] = {}
        self.support: dict[str, float] = {}

        # Create item list during db creation only
        self.item_list: list[str] = []

        self.read_utils(csv_utils)
        self.read_transactions(csv_transactions)

    def read_utils(self, path: str):
        with open(path, 'r') as f:
            lines = f.readlines()

        self.min_util = float('inf')
        self.max_util = float('-inf')

        for line in lines:
            data = line.split(", ")
            label = data[0].strip()
            utility = float(data[1].strip())

            self.min_util = min(self.min_util, utility)
            self.max_util = max(self.max_util, utility)

            self.utils[label] = utility

    def read_transactions(self, path: str):
        with open(path, 'r') as f:
            lines = f.readlines()

        labels_twu = {}

        # Adding values to database
        for line_index in range(len(lines)):
            data = lines[line_index].split(":")

            labels = [item.strip() for item in data[0].split(' ')]
            quantities = [float(util.strip()) for util in data[2].split(' ')]

            twu = 0
            items: list[Item] = []
            for i in range(len(labels)):
                label = labels[i]
                quantity = quantities[i]
                util = self.utils[label]

                # Increment the count of label
                self.support[label] = self.support.get(label, 0) + 1

                twu += quantity * util
                items.append(Item(label, quantity * util))

            # Adding the values of twu of the transaction to labels
            for item in items:
                labels_twu[item.label] = labels_twu.get(item.label, 0) + twu

            self.transactions.append(Transaction(line_index + 1, twu, items))

        # Remove items having labels_twu less than min_util
        revised_labels: set[str] = set()
        for i in range(len(self.transactions)):
            revised_twu = 0
            revised_items: list[Item] = []

            for item in self.transactions[i].items:
                if labels_twu[item.label] >= self.min_util:
                    revised_twu += item.value
                    revised_items.append(item)
                    revised_labels.add(item.label)

            # sort the items with their support
            revised_items.sort(
                reverse=True,
                key=lambda item: self.support[item.label]
            )

            self.transactions[i].items = revised_items
            self.transactions[i].twu = revised_twu

        # Item list is all revised labels in descending order of support
        self.item_list = list(revised_labels)
        self.item_list.sort(
            reverse=True,
            key=lambda item: self.support[item]
        )

    def get_item_list(self):
        return self.item_list
