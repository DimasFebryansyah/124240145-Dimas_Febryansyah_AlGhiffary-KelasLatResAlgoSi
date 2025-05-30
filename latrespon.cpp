#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

// Struktur data untuk buku
struct Book
{
    string title;
    string author;
    int year;
    Book *left;
    Book *right;

    Book(string t, string a, int y) : title(t), author(a), year(y), left(nullptr), right(nullptr) {}
};

// Struktur data untuk stack (undo)
struct Action
{
    string type; // "add" atau "delete"
    Book *book;
    Action *next;

    Action(string t, Book *b) : type(t), book(b), next(nullptr) {}
};

class BookManager
{
private:
    Book *root;
    Action *undoStack;

public:
    BookManager() : root(nullptr), undoStack(nullptr) {}

    ~BookManager()
    {
        clearTree(root);
        clearUndoStack();
    }

    void clearTree(Book *node)
    {
        if (node != nullptr)
        {
            clearTree(node->left);
            clearTree(node->right);
            delete node;
        }
    }

    void clearUndoStack()
    {
        while (undoStack != nullptr)
        {
            Action *temp = undoStack;
            undoStack = undoStack->next;
            delete temp;
        }
    }

    void addBook()
    {
        string title, author;
        int year;

        cout << "\n=== TAMBAH BUKU BARU ===\n";
        cout << "Judul: ";
        getline(cin, title);
        cout << "Pengarang: ";
        getline(cin, author);
        cout << "Tahun Terbit: ";
        cin >> year;
        cin.ignore();

        if (searchBook(title) != nullptr)
        {
            cout << "Buku dengan judul tersebut sudah ada!\n";
            return;
        }

        Book *newBook = new Book(title, author, year);
        root = insert(root, newBook);

        // Catat aksi ke stack undo dengan referensi asli
        pushUndo("add", newBook);

        cout << "Buku berhasil ditambahkan!\n";
    }

    Book *insert(Book *node, Book *newBook)
    {
        if (node == nullptr)
            return newBook;

        if (newBook->title < node->title)
            node->left = insert(node->left, newBook);
        else if (newBook->title > node->title)
            node->right = insert(node->right, newBook);

        return node;
    }

    void displayBooks()
    {
        cout << "\n=== DAFTAR BUKU ===\n";
        cout << left << setw(40) << "Judul"
             << setw(25) << "Pengarang"
             << "Tahun" << endl;
        cout << string(80, '-') << endl;

        if (root == nullptr)
            cout << "Belum ada buku yang tersimpan\n";
        else
            inOrder(root);
    }

    void inOrder(Book *node)
    {
        if (node != nullptr)
        {
            inOrder(node->left);
            cout << left << setw(40) << node->title
                 << setw(25) << node->author
                 << node->year << endl;
            inOrder(node->right);
        }
    }

    void deleteBook()
    {
        string title;
        cout << "\n=== HAPUS BUKU ===\n";
        cout << "Masukkan judul buku yang akan dihapus: ";
        getline(cin, title);

        Book *bookToDelete = searchBook(title);
        if (bookToDelete == nullptr)
        {
            cout << "Buku tidak ditemukan!\n";
            return;
        }

        pushUndo("delete", bookToDelete); // Simpan referensi asli sebelum dihapus
        root = deleteNode(root, title);
        cout << "Buku berhasil dihapus!\n";
    }

    Book *deleteNode(Book *node, const string &title)
    {
        if (node == nullptr)
            return node;

        if (title < node->title)
            node->left = deleteNode(node->left, title);
        else if (title > node->title)
            node->right = deleteNode(node->right, title);
        else
        {
            if (node->left == nullptr)
            {
                Book *temp = node->right;
                delete node;
                return temp;
            }
            else if (node->right == nullptr)
            {
                Book *temp = node->left;
                delete node;
                return temp;
            }

            Book *temp = minValueNode(node->right);
            node->title = temp->title;
            node->author = temp->author;
            node->year = temp->year;
            node->right = deleteNode(node->right, temp->title);
        }
        return node;
    }

    Book *minValueNode(Book *node)
    {
        Book *current = node;
        while (current && current->left != nullptr)
            current = current->left;
        return current;
    }

    Book *searchBook(const string &title)
    {
        return search(root, title);
    }

    Book *search(Book *node, const string &title)
    {
        if (node == nullptr || node->title == title)
            return node;

        if (title < node->title)
            return search(node->left, title);
        return search(node->right, title);
    }

    void undo()
    {
        if (undoStack == nullptr)
        {
            cout << "Tidak ada aksi yang dapat di-undo!\n";
            return;
        }

        Action *lastAction = undoStack;
        undoStack = undoStack->next;

        if (lastAction->type == "add")
        {
            // Undo penambahan = hapus buku dari BST
            Book *bookToDelete = searchBook(lastAction->book->title);
            if (bookToDelete)
            {
                root = deleteNode(root, bookToDelete->title);
                cout << "Undo penambahan buku: " << lastAction->book->title << endl;
            }
        }
        else if (lastAction->type == "delete")
        {
            // Undo penghapusan = tambahkan kembali buku ke BST
            // Buat salinan baru untuk BST (bukan menggunakan pointer dari stack)
            Book *restoredBook = new Book(lastAction->book->title,
                                          lastAction->book->author,
                                          lastAction->book->year);
            root = insert(root, restoredBook);
            cout << "Undo penghapusan buku: " << lastAction->book->title << endl;
        }

        // Hapus action dari stack
        delete lastAction->book; // Hapus buku yang disimpan di stack
        delete lastAction;       // Hapus action itu sendiri
    }

    void pushUndo(string type, Book *book)
    {
        // Selalu buat salinan buku untuk disimpan di stack
        Book *bookCopy = new Book(book->title, book->author, book->year);
        Action *newAction = new Action(type, bookCopy);
        newAction->next = undoStack;
        undoStack = newAction;
    }

    void showMenu()
    {
        cout << "\n=== APLIKASI MANAJEMEN BUKU ===\n";
        cout << "1. Tambah Buku Baru\n";
        cout << "2. Tampilkan Daftar Buku\n";
        cout << "3. Hapus Buku\n";
        cout << "4. Undo Aksi Terakhir\n";
        cout << "5. Keluar\n";
        cout << "Pilihan: ";
    }
};

int main()
{
    BookManager manager;
    int choice;

    do
    {
        manager.showMenu();
        cin >> choice;
        cin.ignore();

        switch (choice)
        {
        case 1:
            manager.addBook();
            break;
        case 2:
            manager.displayBooks();
            break;
        case 3:
            manager.deleteBook();
            break;
        case 4:
            manager.undo();
            break;
        case 5:
            cout << "Terima kasih telah menggunakan aplikasi!\n";
            break;
        default:
            cout << "Pilihan tidak valid. Silakan coba lagi.\n";
        }
    } while (choice != 5);

    return 0;
}
