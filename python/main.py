import inquirer
from inquirer.questions import os
from ECoHUP import getPatterns


def main():
    answers = inquirer.prompt([
        inquirer.List(
            'csv_folder',
            message="Select a csv folder as a input: ",
            choices=os.listdir("./csv"),
        )
    ])

    if answers == None:
        return

    csv_folder = answers["csv_folder"]
    csv_transactions = "./csv/{}/transactions.csv".format(csv_folder)
    csv_utils = "./csv/{}/utils.csv".format(csv_folder)

    # Minimum utility and Minimum Correlation
    min_util = int(input("Enter min util: "))
    min_corr = float(input("Enter min corr: "))

    # Getting the patterns
    result = getPatterns(csv_transactions, csv_utils, min_util, min_corr)
    result.sort()

    print("\nPatterns: ", len(result))
    for pattern in result:
        print(pattern)
    print()


if __name__ == "__main__":
    main()
