/*Γράφω πρώτα τις οδηγίες προς τον προμεταγλωττιστή της C,
οι οποίες εκτελούνται από αυτόν ένα στάδιο πριν αρχίσει η κανονική μεταγλώττιση*/

//Η οδηγία #include επιβάλλει στον προμεταγλωττιστή να συμπεριλάβει κάποιο αρχείο κεφαλίδας (αρχείο που περιέχει συναρτήσεις βιβλιοθήκης)
#include <stdio.h> //Βιβλιοθήκη εισόδου/εξόδου.
#include <stdlib.h> //Βιβλιοθήκη για την εκχώρηση μνήμης, τον έλεγχο διεργασιών κτλπ.
#include <stdbool.h> //Βιβλιοθήκη για τύπους και τιμές boolean.
#include <math.h> //Βιβλιοθήκη μαθηματικών. 
#include <mpi.h> //Βιβλιοθήκη Message Passing Interface (MPI) για παράλληλους υπολογισμούς.

//Η οδηγία #define ορίζει ένα αναγνωριστικό και ένα σύνολο χαρακτήρων που θα αντικαταστήσει αυτό το αναγνωριστικό.
// Ορίζω το μέγεθος του πίνακα (Ν), αν δεν έχει προκαθοριστεί.
#ifndef N //Εάν η σταθερά N δεν έχει οριστεί, προχώρα στην επόμενη γραμμή.
#define N 16 //Ορίζω το N ως 16.
#endif

// Ορίζω την ριζική διεργασία για τις λειτουργίες MPI
#define _ROOT 0  

//Όνομα αρχείου εξόδου για το μητρώο A που προκύπτει.
#ifndef OUT_FILE_NAME //Εάν η σταθερά OUT_FILE_NAME δεν έχει οριστεί, προχώρα στην επόμενη γραμμή.
#define OUT_FILE_NAME "outputA.txt" //Ορίζω το OUT_FILE_NAME ως outputA.txt.
#endif


//MPI communicator για την τοπολογία πλέγματος - grid topology
//Δηλώνω την μεταβλητή τύπου MPI_Comm, grid_comm για τον πληροφοριοδότη/επικοινωνητή MPI στην τοπολογία πλέγματος.
//Ο communicator αποτελεί μια εικονική οντότητα που ορίζει μια ομάδα διεργασιών MPI που μπορούν να επικοινωνούν μεταξύ τους, μέσω συναρτήσεων επικοινωνίας.
//Οι συναρτήσεις επικοινωνίας MPI απαιτούν έναν communicator ως όρισμα για να καθορίσει ποιες διεργασίες συμμετέχουν στην επικοινωνία.
MPI_Comm grid_comm;  

//Μεταβλητές για την αποθήκευση των διαστάσεων του πλέγματος διεργασιών.
int dim1, dim2;

//Δήλωση Συναρτήσεων.
void findMultiples(int n, int* n1, int* n2); 
int** createMatrix(int m, int n); 

//Εκχωρώ μνήμη για τον δισδιάστατο πίνακα.
int** createMatrix(int m, int n) {

    /*Η συνάρτηση void *malloc(unsigned int size), δεσμεύει από το σώρό ένα τμήμα μνήμης (μπλοκ-πλοκάδα), μεγέθους σε bytes, όσο η τιμή της
      παραμέτρου size και επιστρέφει ως τιμή τη διεύθυνση της πρώτης θέσης μνήμης αυτού του μπλοκ */

    //Δημιουργώ δυναμικά έναν πίνακα δεικτών σε ακέραιους αριθμούς χρησιμοποιώντας τον διπλό δείκτη. 
    //Μόλις τον δημιουργήσω μπορώ να εκχωρήσω δυναμικά μνήμη για κάθε γραμμή.
    //Άρα ο πίνακας δεικτών matrix_2d θα κρατήσει τις διευθύνσεις των δυναμικά εκχωρημένων γραμμών (πινάκων ακεραίων) στον δισδιάστατο μητρώο.
    int** matrix_2d = (int**)malloc(m * sizeof(int*)); 
    

    //Δεσμεύω ένα μπλοκ μνήμης μεγέθους m*n και αποκτώ πρόσβαση στα στοιχεία του χρησιμοποιώντας απλό δείκτη σε έναν ακέραιο. 
    //Μετατροπή του δείκτη void που επιστρέφει η συνάρτηση malloc σε δείκτη προς δεδομένα τύπου int
    int* matrix_1d = (int*)malloc(m * n * sizeof(int));
    
    //Έπειτα καθορίζω την σχέση μεταξύ του matrix_2d και του matrix_1d
    //Το matrix_1d είναι το μπλοκ μνήμης με στοιχεία m * n.
    //Το matrix_2d είναι ο πίνακας δεικτών, όπου κάθε δείκτης δείχνει σε μια συγκεκριμένη γραμμή μέσα σε αυτό το μπλοκ.
    //Με τον βρόγχο διασφαλίζω ότι κάθε δείκτης γραμμής στο matrix_2d δείχνει στη πρώτη θέση μνήμης της αντίστοιχης γραμμής στο matrix_1d.
    for (int i = 0; i < m; i++) {
        
        // Για κάθε γραμμή (με ευρετήριο i) στον δισδιάστατο πίνακα:
        // Το matrix_2d[i] είναι ένας δείκτης σε έναν ακέραιο (δείκτης γραμμής).
        // Το &matrix_1d[i * n] υπολογίζει τη διεύθυνση της πρώτης θέσης μνήμης (έναρξης) της αντίστοιχης γραμμής στον 1D.
        // Εκχωρώ το &matrix_1d[i * n] στο matrix_2d[i], ώστε ο δείκτης γραμμής στο matrix_2d να δείχνει τη σωστή θέση μνήμης στον πίνακα 1D.
        // Ουσιαστικά, κάθε δείκτης γραμμής στο 2D δείχνει στη κατάλληλη θέση μνήμης στον 1D.
        matrix_2d[i] = &matrix_1d[i * n];

        
    }
    return matrix_2d; //Επιστρέφω τον δυναμικά εκχωρημένο δισδιάστατο πίνακα.
}


int main(int argc, char* argv[]) {
    
    //Δηλώνω ακέραιες μεταβλητές.
    int my_rank, ierr, num_procs; 
    
    //Δημιουργώ την διεργασία.
    //Αρχικοποιώ τον υπολογισμό και εκκινώ τη βιβλιοθήκη MPI.
    //Επιτρέπω στο πρόγραμμα να χρησιμοποιεί συναρτήσεις MPI, για επικοινωνία μεταξύ διεργασιών και για παράλληλη εκτέλεση/επεξεργασία.
    //Πρέπει να καλείται από κάθε διεργασία MPI ακριβώς μια φορά.
    ierr = MPI_Init(&argc, &argv);
   
    //Επιστρέφω το αναγνωριστικό/αριθμό της διεργασίας.
    //Το MPI_COMM_WORLD είναι σταθερά που ορίζει έναν communicator που περιέχει όλες τις διεργασίες 
    //και μέσω του οποίου ανακτώ την κατάταξη της τρέχουσας διεργασίας.
    //Η κατάταξη είναι μια ακέραια τιμή που προσδιορίζει μοναδικά κάθε διεργασία στον communicator.
    //Ο αριθμός της τρέχουσας διεργασίας (κατάταξή της) επιστρέφεται και αποθηκεύεται στο my_rank.
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    


    //Επιστρέφω το πλήθος των διεργασιών που συμμετέχουν στον communicator.
    //Το πλήθος των διεργασιών αποθηκεύεται στη μεταβλητή num_procs.
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    

    //Με βάση του πλήθους των διεργασιών βρίσκω τις κατάλληλες διαστάσεις για το πλέγμα διεργασιών. 
    //Στον παράλληλο υπολογισμό, όταν κατανέμουμε ένα task (μια εργασία) μεταξύ πολλών διεργασιών, 
    //τις οργανώνουμε σε ένα πλέγμα διεργασιών.
    //Το πλέγμα έχει διεργασία γραμμής dim1 και διεργασία στήλης dim2, όπου dim1 × dim2 = num_procs (συνολικό πλήθος διεργασιών).
    //Κάθε διεργασία ευρετηριάζεται από τη γραμμή και τη στήλη της (i, j), όπου 0 ≤ i < dim1 και 0 ≤ j < dim2.
    //Ο στόχος είναι ο αποτελεσματικός διαχωρισμός των διεργασιών σε γραμμές και στήλες καθώς και
    //πόσες γραμμές (dim1) και στήλες (dim2) χρειάζονται για να φιλοξενήσουν το δεδομένο πλήθος διεργασιών.
    findMultiples(num_procs, &dim1, &dim2);

    
    //Μεταβλητή για την αποθήκευση της νέας κατάταξης (αναγνωριστικού της νέας τρέχουσας διεργασίας) του communicator
    int new_rank = 0;  


    //Δημιουργώ μια δισδιάστατη καρτεσιανή τοπολογία πλέγματος.
    //Σαν ένα πλέγμα όπου κάθε σημείο στον χώρο είναι τοποθετημένο σε ένα κουτί. 
    //Κάθε κουτί συνδέεται με τα γειτονικά του, με οριζόντιες και κάθετες συνδέσεις 
    //και έχει δύο γειτονικά στοιχεία στον άξονα x και δύο στον άξονα y, εκτός από τις άκρες του πλέγματος όπου μπορεί να έχουν λιγότερους γείτονες, 
    //μέσω της δημιουργίας του ακέραιου πίνακα που περιέχει τις διαστάσεις του πλέγματος, δηλαδή το πλήθος των διεργασιών σε κάθε διάσταση.
    int dims[] = { dim1, dim2 };

    //Δημιουργώ τον πίνακα p,ώστε να προσδιορίσω εάν επιτρέπονται συνδέσεις περιτύλιξης σε κάθε διάσταση.
    int periods[2] = { 0, 0 };   //Χωρίς συνδέσεις περιτυλίγματος (περιοδικότητα).
    

    //Επιτρέπω στο MPI να αναδιατάσσει τις διεργασίες για καλύτερη απόδοση.
    int reorder = 1;  


    //Δημιουργώ έναν νέο communicator που όρισα στην αρχή για την επικοινωνία και τον συντονισμό εντός του καρτεσιανού πλέγματος.
    //Ο νέος communicator δέχεται ως όρισματα τις πληροφορίες της καρτεσιανής τοπολογίας:
    //MPI_COMM_WORLD - Ο υπάρχον communicator, αντιπροσωπεύει όλες τις διεργασίες στο παράλληλο πρόγραμμα.
    //2 - Το πλήθος των διαστάσεων στο καρτεσιανό πλέγμα.
    //dims - Πίνακας που περιέχει τις διαστάσεις του πλέγματος (το πλήθος διεργασιών σε κάθε διάσταση).
    //periods - Πίνακας που προσδιορίζει εάν επιτρέπονται συνδέσεις περιτυλίγματος (περιοδικότητα) σε κάθε διάσταση.
    //reorder - Σημαία που υποδεικνύει εάν το MPI μπορεί να αναδιατάξει τις διεργασίες για να βελτιώσει την απόδοση.
    //&grid_comm - Διεύθυνση της μεταβλητής στην οποία θα αποθηκευτεί ο νέος communicator.
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &grid_comm);

    
    // Λαμβάνω τις συντεταγμένες της τρέχουσας διεργασίας στο πλέγμα
    //Ορίζω τον ακέραιο πίνακα για την αποθήκευση των συντεταγμένων (γραμμή και στήλη) της τρέχουσας διεργασίας.
    int coords[2]; 
    //Ανακτώ την το αναγνωριστικό της τρέχουσας διεργασίας εντός του communicator, grid_comm και την αποθηκεύω στην μεταβλητή new_rank.
    MPI_Comm_rank(grid_comm, &new_rank); 
    //Αποθηκεύω τις καρτεσιανές συντεταγμένες (γραμμή και στήλη) της διεργασίας με αναγνωριστικό new_rank εντός communicator στον πίνακα coords.
    MPI_Cart_coords(grid_comm, new_rank, 2, coords); 
    
    //Πλέον ο πίνακας coords περιέχει τα ευρετήρια γραμμών και στηλών της τρέχουσας διεργασίας μέσα στο δισδιάστατο καρτεσιανό πλέγμα.
    //Αυτές οι συντεταγμένες χρησιμοποιούνται για συγκεκριμένους υπολογισμούς και επικοινωνία εντός του πλέγματος.

    //Ξεκινώ την χρονομέτρηση της εκτέλεσης - ενεργοποιώ το υποθετικό χρονόμετρο
    //Αποθηκεύω την επιστρεφόμενη τιμή χρόνου στη μεταβλητή startTime, ώστε
    //αργότερα να υπολογίσω τη διάρκεια της εκτέλεσης αφαιρώντας την ώρα έναρξης από την ώρα λήξης.
    double startTime = MPI_Wtime(); 
    

    //Εκχώρηση μνήμης για τους δισδιάστατους πίνακες B και C
    int** B = createMatrix(N, N);
    int** C = createMatrix(N, N);
    int** A = NULL;  //Ο πίνακας Α θα χρησιμοποιηθεί μόνο από τη ριζική διεργασία.

    //Η ριζική διεργασία αρχικοποιεί τους πίνακες B και C με τιμές που παρέχει ο χρήστης
    //Ελέγχω εάν η τρέχουσα διεργασία (new_rank) είναι η ριζική διεργασία (new_rank=0).
    if (new_rank == _ROOT) {
        A = createMatrix(N, N); //Εκχωρώ μνήμη για τον πίνακα A, που χρησιμοποιείται μόνο από την ριζική διεργασία.

        //Ζητάω από τον χρήστη να εισάγει τιμές για τον πίνακα B και C.
        printf("Please provide %d x %d Matrix B\n", N, N);

        //Ο χρήστης εισάγει τιμές για κάθε στοιχείο του πίνακα Β, όμοια και για τον C.
        //Αρχικοποιώ τους πίνακες B και C με τιμές που παρείχε ο χρήστης.
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                scanf("%d", &B[i][j]);  //Διαβάζω τις τιμές του χρήστη για τον πίνακα B[i][j]
            }
        }
        printf("Read Completed for Matrix B\n");

        printf("Please provide %d x %d Matrix C\n", N, N);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                scanf("%d", &C[i][j]);  //Διαβάζω τις τιμές του χρήστη για τον πίνακα C[i][j]
            }
        }
        printf("Read Completed for Matrix C\n");
    }




    //Υπολογίζω το μέγεθος του υποπλέγματος που θα χειριστεί κάθε διεργασία μέσα στο δισδιάστατο καρτεσιανό πλέγμα.
    //Δηλαδή καθορίζω το τμήμα του συνολικού πλέγματος στο οποίο θα λειτουργεί κάθε διεργασία. Αυτός ο τεμαχισμός/διαίρεση,
    //διασφαλίζει ότι η εργασία κατανέμεται ομοιόμορφα μεταξύ των διεργασιών.
    
    
    //Διαιρώ το συνολικό μέγεθος του πλέγματος N με το πλήθος των γραμμών dim1.
    //Προκύπτει το πλήθος των εγγραφών/στοιχείων κατά μήκος του άξονα x (γραμμές) που θα χειριστεί κάθε διεργασία.
    int grid_size_x_entries = N / dim1;
    
    //Όμοια διαιρώ το συνολικό μέγεθος του πλέγματος N με το πλήθος των στηλών dim2.
    //Προκύπτει το πλήθος των εγγραφών/στοιχείων κατά μήκος του άξονα y (στήλες) που θα χειριστεί κάθε διεργασία.
    int grid_size_y_entries = N / dim2;


         
    //Υπολογίζω τις αρχικές συντεταγμένες του υποπλέγματος για κάθε διεργασία.
    //Δηλαδή τα ευρετήρια όπου κάθε διεργασία θα ξεκινήσει τον υπολογισμό της εντός ξεχωριστού τμήματος στο πλέγμα.

   
    //Πολλαπλασιάζω την συντεταγμένη x της διεργασίας με το μέγεθος του υποπλέγματος κατά μήκος του άξονα x. 
    //Προκύπτει το αρχικό ευρετήριο της γραμμής για την τρέχουσα διαδικασία.
    int start_x = coords[0] * grid_size_x_entries;
   
    //Πολλαπλασιάζω την συντεταγμένη y της διεργασίας με το μέγεθος του υποπλέγματος κατά μήκος του άξονα y. 
    //Προκύπτει το αρχικό ευρετήριο της στήλης για την τρέχουσα διαδικασία.
    int start_y = coords[1] * grid_size_y_entries;
    

    //Υπολογίζω τυχόν επιπλέον εγγραφές για τις διεργασίες ορίων.
    //Σε περίπτωση που οι γραμμές/στήλες στην διάσταση x/y αντίστοιχα δεν διαιρούνται εξίσου με το πλήθος των διεργασιών, 
    //τότε προσθέτω επιπλέον γραμμές/στήλες αυτόματα στην οριακή (τελευταία) διεργασία.

    
    //Εάν η τρέχουσα διεργασία βρίσκεται στο όριο (τελευταία θέση) κατά μήκος του άξονα x, τότε
    //Υπολογίζω το υπόλοιπο, διαιρώντας το συνολικό μέγεθος του πλέγματος με την συντεταγμένη x (ευρετήριο γραμμής) της τρέχουσας διεργασίας μέσα στο πλέγμα.
    //Το υπόλοιπο αντιπροσωπεύει τυχόν επιπλέον εγγραφές πέρα από το ομοιόμορφα κατανεμημένο μέγεθος υποπλέγματος.
    //Προσθέτω το υπόλοιπο στο μέγεθος του υπο-πλέγματος κατά μήκος του άξονα x.
    //Προσαρμόζω το μέγεθος του υποπλέγματος (προσθέτοντας γραμμές στο όριο) που χειρίζεται κάθε διεργασία κατά μήκος του άξονα x ώστε 
    //να λάβω υπόψη τυχόν επιπλέον εγγραφές που απαιτούνται στο όριο, εξαιτίας ανομοιόμορφης κατανομής της εργασίας και κατανέμω ισόποσα τον φόρτο εργασίας.
    if (coords[0] == dim1 - 1) grid_size_x_entries += N % dim1;
    
     
    //Κατά τον ίδιο τρόπο εάν η τρέχουσα διεργασία βρίσκεται στο όριο κατά μήκος του άξονα y, τότε
    //Υπολογίζω το υπόλοιπο, διαιρώντας το συνολικό μέγεθος του πλέγματος με την συντεταγμένη y (ευρετήριο στήλης) της τρέχουσας διεργασίας μέσα στο πλέγμα.
    //Προσθέτω το υπόλοιπο στο μέγεθος του υπο-πλέγματος κατά μήκος του άξονα y.
    if (coords[1] == dim2 - 1) grid_size_y_entries += N % dim2;
    

    //Υπολογίζω τις τελευταίες συντεταγμένες (αποκλειστικά) του υποπλέγματος.
    int end_x = start_x + grid_size_x_entries;
    int end_y = start_y + grid_size_y_entries;


    //Εκχώρηση μνήμης για το τοπικό τμήμα του πίνακα A, που θα υπολογίζει η κάθε διεργασία.
    //Διασφαλίζω ότι κάθε διεργασία έχει το δικό της τμήμα του πίνακα A για να εργαστεί κατά τη διάρκεια του υπολογισμού.
    int** APart = createMatrix(grid_size_x_entries, grid_size_y_entries);

    //Μεταδίδω τα μητρώα Β και C σε όλες τις διεργασίες.
    //Με την εντολή MPI_Bcast στέλνω το ίδιο τμήμα δεδομένων σε όλες τις άλλες διεργασίες. 
    //Η root διεργασία μέσω της MPI_Bcast αντιγράφει/στέλνει τα στοιχεία (δεδομένα) της σε όλες τις άλλες διεργασίες,
    //εντός του ίδιου communicator, grid_comm. Με την προυπόθεση ότι πρέπει να καλείται από όλες τις διεργασίες με τα ίδια ορίσματα.

    
    //Μεταδίδω τα περιεχόμενα του πίνακα B από τη ριζική διεργασία σε όλες τις άλλες διεργασίες στον communicator grid_comm. 
    //&B[0][0] - Η μετάδοση ξεκινά από αυτήν τη θέση μνήμης, την διεύθυνση του πρώτου στοιχείου του πίνακα B.
    //N * N - Ο συνολικός αριθμός στοιχείων στον πίνακα B, καθώς είναι N x N πίνακας.
    //MPI_INT - Ο τύπος δεδομένων των στοιχείων στον πίνακα B.
    //_ROOT - Η κατάταξη της διεργασίας που μεταδίδει τα δεδομένα (η ριζική διεργασία).
    //grid_comm - Ο communicator μέσω του οποίου εκτελείται η λειτουργία εκπομπής.
    MPI_Bcast(&B[0][0], N * N, MPI_INT, _ROOT, grid_comm);
    
    //Ομοίως, μεταδίδω τα περιεχόμενα του πίνακα C χρησιμοποιώντας τα ίδια ορίσματα.
    MPI_Bcast(&C[0][0], N * N, MPI_INT, _ROOT, grid_comm);
    
    
    /*
    Συνοπτικά, η ριζική διεργασία εκκινεί τη μετάδοση έχοντας τα δεδομένα στην τοπική της μνήμη. 
    Τα δεδομένα της ριζικής διεργασίας (ξεκινάνε από τη θέση μνήμης &B[0][0] και &C[0][0]) αντιγράφονται στις αντίστοιχες θέσεις μνήμης
    (τοπική μνήμη) όλων των άλλων διεργασιών στον communicator. Διασφαλίζω ότι όλες οι διεργασίες έχουν πανομοιότυπα αντίγραφα (λαμβάνουν τα ίδια δεδομένα)
    των πινάκων B και C για τον συγχρονισμό και την κοινή χρήση δεδομένων μεταξύ των διεργασιών.
    */


    //Εκτελώ τον πολλαπλασιασμό των μητρώων B και C στο τοπικό υποπλέγμα.
    //Ορίζω το υπο-πλέγμα από τις συντεταγμένες (start_x, start_y) έως (end_x - 1, end_y - 1)
    /*
    Για κάθε κελί (i, j) στο υποπλέγμα:
    Αρχικοποίώ την προσωρινή μεταβλητή temp σε μηδέν.
    Χρησιμοποιώ έναν άλλο βρόγχο για να υπολογίσω το εσωτερικό γινόμενο της κ-στης γραμμής του πίνακα B και της κ-στης στήλης του πίνακα C.
    Συσσωρεύω το αποτέλεσμα στο temp.
    Εκχωρώ την τιμή temp στο αντίστοιχο κελί (i - start_x, j - start_y) στον τοπικό υπομητρώο APart (τμήμα του πίνακα A), που χειρίζεται η κάθε διεργασία.
    Καθώς κάθε διεργασία έχει το δικό της τμήμα του πίνακα A για να εργαστεί κατά τη διάρκεια του υπολογισμού.
    Στο τέλος το υπομητρώο APart θα περιέχει τα υπολογισμένα γινόμενα για το καθορισμένο υποπλέγμα.
    */
    for (int i = start_x; i < end_x; i++) {  //Ο εξωτερικός βρόγχος επαναλαμβάνεται πάνω από τις γραμμές του υποπλέγματος.
        for (int j = start_y; j < end_y; j++) {  // Ο εσωτερικός βρόχος επαναλαμβάνεται πάνω από τις στήλες του υποπλέγματος.
            int temp = 0;
            for (int k = 0; k < N; k++)
                temp += B[i][k] * C[k][j];
            APart[i - start_x][j - start_y] = temp;
        }
    }

    

    // Η ριζική διεργασία συγκεντρώνει τα υπολογισμένα υποπλέγματα από όλες τις διεργασίες, συμπεριλαμβανομένης της ίδιας.
    if (my_rank == _ROOT)
    {
        // Συντεταγμένες της ριζικής διεργασίας στην τοπολογία πλέγματος.
        //Ο πίνακας coords_loc θα περιέχει τους δείκτες γραμμών και στηλών της ριζικής διεργασίας.
        int coords_loc[2];
        
        //Ανακτώ τις καρτεσιανές συντεταγμένες της ριζικής διεργασίας με κατάταξη 0 εντός του communicator (grid_comm). 
        //2 - Ο αριθμός των διαστάσεων στην καρτεσιανή τοπολογία.
        //coords_loc - Ο πίνακας όπου θα αποθηκευτούν οι προκύπτουσες συντεταγμένες (τα ευρετήρια γραμμής και στήλης).
        MPI_Cart_coords(grid_comm, 0, 2, coords_loc);
        
        //Υπολογίζω το τελικό μέγεθος του υποπλέγματος για τη ριζική διεργασία, 
        //λαμβάνοντας υπόψη τυχόν επιπλέον γραμμές ή στήλες λόγω ανομοιόμορφης διαίρεσης.
        int grid_size_x_entries_loc = grid_size_x_entries;
        int grid_size_y_entries_loc = grid_size_y_entries;

        //Εάν η ριζική διεργασία βρίσκεται στην τελευταία γραμμή, προσαρμόζω το πλήθος των γραμμών 
        //προσθέτοντας το υπόλοιπο της διαίρεσης του N με το dim1.
        if (coords_loc[0] == dim1 - 1)
        {
            grid_size_x_entries_loc += N % dim1; //Προσθέτω επιπλέον γραμμές στο μπλοκ της τελευταίας γραμμής.
        }
        //Όμοια για τις στήλες.
        if (coords_loc[1] == dim2 - 1)
        {
            grid_size_y_entries_loc += N % dim2; //Προσθέτω επιπλέον στήλες στο μπλοκ της τελευταίας στήλης.
        }


        //Η ριζική διεργασία αντιγράφει απευθείας το υπολογιζόμενο υποπλέγμα APart στον πλήρη πίνακα A.
        //Οι ένθετοι βρόχοι επαναλαμβάνονται πάνω από το υποπλέγμα και κάθε κελί (i, j) στο υποπλέγμα αντιστοιχεί στο κελί (i, j) στον πλήρη πίνακα A.
        for (int i = 0; i < grid_size_x_entries_loc; i++)
        {
            for (int j = 0; j < grid_size_y_entries_loc; j++)
            {
                A[i][j] = APart[i][j];
            }
        }


        //Αρχικοποιώ τις μεταβλητές next_x και next_y για να παρακολουθήσω πού θα τοποθετηθεί το επόμενο υπο-πλέγμα στον πλήρη πίνακα Α.
        //Μετά την αντιγραφή του υπο-πλέγματος της ριζικής διεργασίας, το επόμενο υπό-πλέγμα θα ξεκινήσει από τη γραμμή next_x και τη στήλη next_y.
        int next_x = 0;
        int next_y = grid_size_y_entries_loc;
       

        //Επαναλαμβάνω για τις υπόλοιπες διεργασίες (από το 1 έως το num_procs - 1) για να λάβω και να συναρμολογήσω τα υπό-πλέγμα τους.
        //Πρόκειται για διεργασίες που είναι υπεύθυνες για τον υπολογισμό των αντίστοιχων υποπλεγμάτων τους.
        for (int rank = 1; rank < num_procs; ++rank)
        {
            //Για την τρέχουσα διεργασία, ανακτώ τις συντεταγμένες του πλέγματος (coords_loc).
            MPI_Cart_coords(grid_comm, rank, 2, coords_loc);

            //Προσαρμόζω το μέγεθος του υποπλέγματος εάν αυτή η διεργασία βρίσκεται στην άκρη του πλέγματος.
            int grid_size_x_entries_loc = grid_size_x_entries;
            int grid_size_y_entries_loc = grid_size_y_entries;
            
            //Εάν η τρέχουσα διεργασία βρίσκεται στην τελευταία γραμμή.
            if (coords_loc[0] == dim1 - 1)
            {
                grid_size_x_entries_loc += N % dim1; //Προσαρμόζω το πλήθος των γραμμών για επιπλέον γραμμές.
            }
            
            //Εάν η τρέχουσα διεργασία βρίσκεται στην τελευταία στήλη.
            if (coords_loc[1] == dim2 - 1)
            {
                grid_size_y_entries_loc += N % dim2; //Προσαρμόζω το πλήθος των στηλων για επιπλέον στήλες.
            }


            //Εκχωρώ έναν προσωρινό πίνακα για να λάβω το υποπλέγμα από την τρέχουσα διεργασία.
            int** A_temp = createMatrix(grid_size_x_entries_loc, grid_size_y_entries_loc);
            

            //Λαμβάνω το υποπλέγμα από την τρέχουσα διεργασία (καθορίζεται από την κατάταξη) και τα αποθηκεύω στον τοπικό πίνακα A_temp. 
            //Πιο αναλυτικά η συνάρτηση MPI_Recv περιλαμβάνει δεδομένα για μια διεργασία MPI που έχουν αποσταλεί από μια άλλη διεργασία.
            /*
            &A_temp[0][0] - Καθορίζω τη θέση μνήμης όπου θα αποθηκευτούν τα ληφθέντα δεδομένα. 
            grid_size_x_entries_loc * grid_size_y_entries_loc - Ο συνολικός αριθμός στοιχείων προς λήψη (το μέγεθος του υποπλέγματος), 
            που υπολογίζεται από την τρέχουσα διεργασία.
            MPI_INT - Ο τύπος δεδομένων των στοιχείων που λαμβάνονται.
            rank - Η κατάταξη της διεργασίας αποστολής (η διεργασία από την οποία λαμβάνονται δεδομένα).
            999 - Μια μοναδική ετικέτα ή αναγνωριστικό μηνύματος. Βοηθά στην αντιστοίχιση των απεσταλμένων δεδομένων με την αντίστοιχη λειτουργία λήψης.
            MPI_STATUS_IGNORE - Αγνοεί τις πληροφορίες κατάστασης που επιστρέφονται από τη συνάρτηση MPI_Recv. 
            */
            MPI_Recv(&A_temp[0][0], grid_size_x_entries_loc * grid_size_y_entries_loc, MPI_INT, rank, 999, grid_comm, MPI_STATUS_IGNORE);
            

            //Αντιγράφω το υπο-πλέγμα που έλαβα στην κατάλληλη θέση στον πλήρη πίνακα A.
            //Οι ένθετοι βρόχοι επαναλαμβάνονται πάνω από το υπο-πλέγμα και κάθε κελί (i, j) στο υποπλέγμα 
            //αντιστοιχεί στο κελί (next_x + i, next_y + j) στον πλήρη πίνακα A.
            for (int i = 0; i < grid_size_x_entries_loc; i++)
            {
                for (int j = 0; j < grid_size_y_entries_loc; j++)
                {
                    A[next_x + i][next_y + j] = A_temp[i][j];
                }
            }
            

            //Ελευθερώνω τη μνήμη που έχει εκχωρηθεί για τον προσωρινό πίνακα.
            //Με αυτόν τον τρόπο διαχειρίζομαι αποτελεσματικά την μνήμη. 
            //Μετά την αντιγραφή του ληφθέντος υποπλέγματος στο A_temp, η ριζική διεργασία απελευθερώνει τη μνήμη για να αποτρέψει διαρροές μνήμης.
            free(A_temp[0]); //Aπελευθερώνω τους πόρους που χρησιμοποιούνται από την πρώτη γραμμή του πίνακα.
            free(A_temp); //Απελευθερώνω ολόκληρο το μπλοκ μνήμης του προσωρινού πίνακα A_temp.
   
            

            //Ενημερώνω τη θέση για την τοποθέτηση του επόμενου υποπλέγματος στον πλήρη πίνακα.
            next_y += grid_size_y_entries_loc;
            if (next_y >= N) //Εάν η θέση της στήλης υπερβαίνει το μέγεθος του πίνακα, 
            {
                next_y = 0; //την επαναφέρω στο 0, ώστε να διασφαλίσω ότι το επόμενο υπο-πλέγμα ξεκινά από την αριστερή στήλη.
                next_x += grid_size_x_entries_loc; //την μετακινώ στο επόμενο μπλοκ γραμμής
            }
        }
    }
    else
    {
        //Οι μη ριζικές διεργασίες στέλνουν το υπολογιζόμενο υποπλέγμα τους στη ριζική διεργασία.
        //Όταν μια διεργασία αποστολής καλέσει την MPI_Send για να στείλει ένα μήνυμα στην διεργασία λήψης,
        //η υλοποίηση MPI θα αποθηκεύσει το μήνυμα μέχρι να κληθεί από την ριζική διεργασία η MPI_Recv, ώστε να το λάβει.
        //Όταν αποστέλλονται πολλά μηνύματα πριν κληθεί η MPI_Recv η σειρά με την οποία λαμβάνονται τα μηνύματα από την διεργασία
        //λήψης θα αντιστοιχή στην σειρά που κλήθηκαν οι συναρτήσεις MPI_Send από τις διεργασίες αποστολής.
        //Έτσι επιτυγχάνω αποτελεσματική επικοινωνία και συγχρονισμό δεδομένων μεταξύ πολλαπλών διεργασιών.
        /*
        Η κάθε διεργασία αποστολής συσκευάζει:
        &APart[0][0] - Καθορίζω τη θέση μνήμης από την οποία θα σταλούν τα δεδομένα (τον buffer). 
        grid_size_x_entries * grid_size_y_entries - Ο συνολικός αριθμός στοιχείων προς αποστολή (το μέγεθος του υποπλέγματος),
        που υπολογίζεται από την τρέχουσα διεργασία.
        MPI_INT - Ο τύπος δεδομένων των στοιχείων που αποστέλλονται.
        _ROOT - Η κατάταξη της διεργασίας προορισμού (ριζική διεργασία).
        999 - Μια μοναδική ετικέτα/αναγνωριστικό μηνύματος. Βοηθά στην αντιστοίχιση των απεσταλμένων δεδομένων με την αντίστοιχη λειτουργία λήψης.
        */
        MPI_Send(&APart[0][0], grid_size_x_entries * grid_size_y_entries, MPI_INT, _ROOT, 999, grid_comm);
        
    }


    //Η ριζική διεργασία γράφει τον τελικό πίνακα A σε αρχείο.
    if (new_rank == _ROOT) {

        //Ανοίγω το αρχείο OUT_FILE_NAME σε λειτουργία εγγραφής. 
        //Η fopen επιστρέφει έναν δείκτη τύπου FILE ο οποίος πρέπει να αποδοθεί στην μεταβλητή fout ίδιου τύπου,
        //αυτός ο τύπος προσδιορίζει το κανάλι επικοινωνίας και με την βοηθειά του αναφερόμαστε στο συγκεγκριμένο αρχείο.   
        FILE* fout = fopen(OUT_FILE_NAME, "w");

        //H fprintf εγγράφει στο αρχείο που καθορίζεται από το δείκτη fout τον πίνακα A και επιστρέφει τον αριθμό των χαρακτήρων που έγραψε στο αρχείο.
        fprintf(fout, "Matrix A:\n");
        for (int i = 0; i < N; i++) { //Οι ένθετοι βρόχοι επαναλαμβάνονται σε ολόκληρο τον πίνακα A και κάθε κελί (i, j) στον πίνακα:
            for (int j = 0; j < N; j++) {
                fprintf(fout, "%d ", A[i][j]); //Η τιμή του A[i][j] και ένα κενό μετά από την κάθε τιμή γράφονται στο αρχείο.
            }
            fprintf(fout, "\n"); //Αφού εγγράψω όλα τα στοιχεία σε μια γραμμή, προστίθεται ο χαρακτήρας αλλαγής γραμμής για να μετακινηθώ στην επόμενη γραμμή.
        }
        //Κλείνω το αρχείο και μέσω της fclose γράφω στο αρχείο όλα τα δεδομένα που βρίσκονται στην περιοχή ενδιάμεσης αποθήκευσης (buffer),
        //πριν κλείσει το αρχείο. Καθώς όταν γράφω δεδομένα στο αρχείο αυτά καταχωρίζονται πρώτα στον buffer και όταν γεμίσει τότε
        //εγγράφονται πραγματικά στο αρχείο.
        fclose(fout);
        
        printf("Matrix A written to file %s\n", OUT_FILE_NAME);
        //Εκτυπώνω το συνολικό χρόνο που χρειάζεται για τον υπολογισμό και το IO.
        printf("Total time taken: %g secs\n", MPI_Wtime() - startTime);
    }


    //Τώρα απελευθερώνω την μνήμη, για να αποφύγω διαρροές με τον εξής τρόπο:
    //Πρώτα ελευθερώνω τη μνήμη στην οποία αναφέρονται οι 2D δείκτες, δηλαδή το συνεχόμενο μπλοκ μνήμης.
    //Στη συνέχεια, ελευθερώνω τους ίδιους τους 2D δείκτες.
    //Ελευθερώνω την εκχωρημένη μνήμη για τους πίνακες B, C και APart
    free(B[0]); //Ελευθερώνω το συνεχόμενο μπλοκ μνήμης για τον πίνακα Β.
    free(B);    //Ελευθερώνω τον πίνακα δεικτών για τον πίνακα Β.
    free(C[0]); //Ελευθερώνω το συνεχόμενο μπλοκ μνήμης για τον πίνακα C.
    free(C);    //Ελευθερώνω τον πίνακα δεικτών για τον πίνακα C.
    free(APart[0]); //Ελευθερώνω το συνεχόμενο μπλοκ μνήμης για το τοπικό τμήμα του πίνακα A.
    free(APart);    //Ελευθερώνω τον πίνακα δεικτών για το τοπικό τμήμα του πίνακα A.


    //Εάν η τρέχουσα διεργασία είναι η ρίζα, ελευθερώνω επίσης τη μνήμη που έχει εκχωρηθεί για τον πλήρη πίνακα A.
    if (new_rank == _ROOT) {
        free(A[0]); //Ελευθερώνω το συνεχόμενο μπλοκ μνήμης για τον πίνακα A.
        free(A);    //Ελευθερώνω τον πίνακα δεικτών για τον πίνακα Α.
    }
    //Τερματίζω τον υπολογισμό και κάνω εκαθάριση στην χρησιμοποιούμενη υλοποίηση.
    //H MPI_Finalize πρέπει να καλέιται από κάθε διεργασία MPI ακριβώς μια φορά.
    MPI_Finalize();
    return 0;
}


//Συνάρτηση για την εύρεση δύο παραγόντων του n έτσι ώστε να είναι όσο το δυνατόν πιο κοντά ο ένας στον άλλο.
//Χρησιμοποιώ τους παράγοντες για να προσδιορίσω τις διαστάσεις του πλέγματος διεργασιών, βρίσκοντας παράγοντες του δεδομένου αριθμού n.
/* Παράδειγμα 
Έστω ότι καλώ αυτή τη συνάρτηση με n = 12:
Το sqrt_n είναι 3 (αφού √12 ≈ 3,46).
Ο βρόχος ελέγχει τους παράγοντες που ξεκινούν από το 3.
Βρίσκει ότι το 3 και το 4 είναι παράγοντες του 12 (3 × 4 = 12).
Η συνάρτηση θέτει *n1 = 3 και *n2 = 4.
*/
void findMultiples(int n, int* n1, int* n2) {
    
    //Υπολογίζω την τετραγωνική ρίζα του n που είναι τιμή κινητής υποδιαστολής, για να διασφαλίσω ακριβείς υπολογισμούς.
    //Μετατρέπω μέσω casting το αποτέλεσμα κινητής υποδιαστολής σε ακέραιο κόβωντας το δεκαδικό μέρος και το αποθηκεύω στην μεταβλητή sqrt_n.
    int sqrt_n = (int)sqrt((double)n); 
         
    //Ο βρόχος ξεκινά από το sqrt_n και επαναλαμβάνεται προς τα πίσω.
    for (int i = sqrt_n; i >= 1; i--) {
        //Για κάθε i:
        //Eάν το υπόλοιπο (modulo) της διαίρεσης του n με τo i είναι ίσο με μηδέν, τότε αυτό σημαίνει ότι το i είναι παράγοντας του n.
        if (n % i == 0) {
            //Οι παράγοντες αποθηκεύονται στις μεταβλητές n1 και n2.
            *n1 = i;
            *n2 = n / i;
            return; //Επιστρέφουμε το πρώτο ζεύγος παραγόντων.
        }
    }
}