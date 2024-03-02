import inquirer
import time
from ECoHUP import getPatterns
import tabulate


def main():
    dataset_path = "./dataset/foodmart.txt"

    # Minimum utility and Minimum Correlation
    min_util = float(input("Enter min util: "))
    min_corr = float(input("Enter min corr: "))

    # Getting the patterns
    result = getPatterns(dataset_path, min_util, min_corr)

    result.sort(key=lambda x: (len(x), x))
    print(tabulate.tabulate([[item] for item in result], ["Items"], "rounded_outline"))
    print("\nPatterns: ", len(result))
    print()

    # Waiting the program
    input("Enter to continue...")

    # Find the unique items
    unique_items = sorted(
        list(set([item for row in result for item in row.split(" ")]))
    )

    answers = inquirer.prompt(
        [
            inquirer.Checkbox(
                "items",
                message="Items to Pair",
                choices=unique_items,
            ),
        ]
    )

    if answers == None:
        return

    compulsoryItems: set[str] = set(answers["items"])
    filtered_items = []
    for itemSet in result:
        is_subset = compulsoryItems.issubset(set(itemSet.split(" ")))
        if is_subset:
            filtered_items.append([itemSet])

    print("Filtered Item Set: ")
    print(tabulate.tabulate(filtered_items, ["Filtered Items"], "rounded_outline"))


def test():
    dataset_path = "./csv/foodmart/transactions.txt"
    min_util: float = 775581.0 * 0.001
    min_corr: float = 0.2

    pow = 1
    for _ in range(1):
        start_time = time.time()

        new_min_util = min_util * pow
        result = getPatterns(dataset_path, new_min_util, min_corr)
        end_time = time.time()
        duration = end_time - start_time

        print("Files:", dataset_path)
        print("Min Util: {}, Min Corr: {}".format(new_min_util, min_corr))
        print("Patterns: ", len(result))
        print("Duration: ", duration)

        pow = pow * 10

        print("#############\n")


def binary_search():
    low = 0
    high = 387790

    dataset_path = "./dataset/foodmart.txt"
    min_corr: float = 0.3

    required = 257

    while low <= high:
        new_min_util = (low + high) // 2

        start_time = time.time()
        result = getPatterns(dataset_path, new_min_util, min_corr)
        end_time = time.time()
        duration = end_time - start_time

        print("Files:", dataset_path)
        print("Min Util: {}, Min Corr: {}".format(new_min_util, min_corr))
        print("Patterns: ", len(result))
        print("Duration: ", duration)

        print("#############\n")

        count = len(result)

        if count > required:
            low = new_min_util + 1
        elif count < required:
            high = new_min_util - 1
        else:
            print(new_min_util, ", Reached")
            break


if __name__ == "__main__":
    main()
