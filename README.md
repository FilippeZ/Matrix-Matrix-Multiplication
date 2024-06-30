# Matrix-Matrix-Multiplication
Τοπολογία Δακτυλίου (Ring)
Ο κώδικας αποτελεί μια αποτελεσματική προσέγγιση στον παράλληλο πολλαπλασιασμό μητρώων χρησιμοποιώντας MPI (Message Passing Interface). Ο κώδικας πολλαπλασιάζει δύο μητρώα, B και C, για να παραγάγει το μητρώο A που προκύπτει, κατανέμοντας τον υπολογισμό σε πολλαπλές διεργασίες, οργανωμένες σε μια τοπολογία δακτυλίου. Αυτή η προσέγγιση αξιοποιεί τις δυνατότητες παράλληλης επεξεργασίας του MPI για τη βελτίωση της απόδοσης με τις παραδοσιακές ακολουθιακές μεθόδους.

Διαμόρφωση Τοπολογίας
Δημιούργησα μια τοπολογία δακτυλίου για τις διαδικασίες υπολογισμού, ώστε να επικοινωνούν μεταξύ τους κατά τη διάρκεια της εργασίας τους, χρησιμοποιώντας το `MPI_Cart_create`, διαμορφώνοντας τις διεργασίες σε μια μονοδιάστατη κυκλική δομή. Δηλαδή, οι διεργασίες είναι διατεταγμένες σε κύκλο (σαν δαχτυλίδι), όπου κάθε διεργασία έχει έναν γείτονα στα αριστερά και έναν άλλο στα δεξιά. Η τοπολογία δακτυλίου επιλέγεται για την απλότητα και την αποτελεσματικότητά της στη διαδοχική μετάδοση δεδομένων που περνούν από τη μια διαδικασία στην άλλη. Αυτή η τοπολογία είναι ιδιαίτερα κατάλληλη για λειτουργίες όπου κάθε διεργασία χρειάζεται να επικοινωνεί με τον προκάτοχο και τον διάδοχο (γειτονικές διεργασίες) της με τρόπο που μοιάζει με αλυσίδα.

Βελτιστοποίηση μνήμης
Χρησιμοποιώ αποτελεσματικά τη μνήμη και την βελτιστοποιώ μέσω προσεκτικής κατανομής και διαχείρισης των μητρώων. Υπάρχουν πολλές διεργασίες που λειτουργούν μαζί. Κάθε διεργασία εκχωρεί μνήμη μόνο για το τμήμα των πινάκων για το οποίο είναι υπεύθυνη, μειώνοντας το συνολικό αποτύπωμα μνήμης. Συγκεκριμένα, για τον πίνακα Β, κάθε διεργασία εκχωρεί μνήμη για ένα υπο-μητρώο (BPart) που αντιστοιχεί στις γραμμές που είναι υπεύθυνη για την επεξεργασία. Αυτή η τοπική κατανομή ελαχιστοποιεί τη χρήση μνήμης, επειδή κάθε διεργασία περιέχει μόνο ό,τι χρειάζεται και διασφαλίζει ότι οι διεργασίες δεν εκχωρούν μνήμη για δεδομένα που δεν χρειάζονται, οδηγώντας σε πιο αποτελεσματική χρήση των πόρων.

Παράλλος Χαρακτήρας και Κατανομή Δεδομένων
Επιτυγχάνω παραλληλισμό μέσω του τεμαχισμού της εργασίας πολλαπλασιασμού μητρώων μεταξύ πολλαπλών διεργασιών.  Η ριζική διεργασία αρχικοποιεί τα μητρώα B και C και στη συνέχεια τεμαχίζει το μητρώο Β και διανέμει τμήματα του σε άλλες διεργασίες στον δακτύλιο χρησιμοποιώντας το MPI_Scatter. Κάθε διεργασία λαμβάνει ένα ξεχωριστό σύνολο γραμμών από το μητρώο Β για να εργαστεί και λειτουργεί ανεξάρτητα πάνω σε αυτό το σύνολο. Μετά από αυτό, το μητρώο C μεταδίδεται σε όλες τις διεργασίες χρησιμοποιώντας το MPI_Bcast, καθώς κάθε διεργασία απαιτεί το πλήρη μητρώο C για να εκτελέσει το μέρος του πολλαπλασιασμού του. Αυτό το σχήμα διανομής δεδομένων επιτρέπει σε όλες τις διεργασίες να λειτουργούν ταυτόχρονα σε διαφορετικά μέρη του υπολογισμού, ενισχύοντας την παράλληλη απόδοση.

Απόδοση
Για την υλοποίηση του κώδικα έκανα αρκετές εκτιμήσεις απόδοσης:
Εξισορρόπηση φορτίου: Το πρόγραμμα εξασφαλίζει ομοιόμορφη κατανομή του φόρτου εργασίας μεταξύ των διαδικασιών τεμαχίζοντας το μητρώο Β σε ίσα μέρη. Αυτή η εξισορρόπηση φορτίου είναι ζωτικής σημασίας για την επίτευξη βέλτιστης παράλληλης απόδοσης, καθώς αποτρέπει ορισμένες διεργασίες από το να είναι αδρανείς ενώ άλλες εξακολουθούν να λειτουργούν.
Επιβάρυνση επικοινωνίας: Η τοπολογία δακτυλίου έχει σχεδιαστεί για να ελαχιστοποιεί την επιβάρυνση της επικοινωνίας. Με την οργάνωση διεργασιών σε έναν δακτύλιο, κάθε διεργασία επικοινωνεί μόνο με δύο γείτονες (εκτός από τη ριζική διεργασία κατά τη διάρκεια των λειτουργιών scatter και gather), μειώνοντας την πολυπλοκότητα και την καθυστέρηση της ανταλλαγής δεδομένων.
Υπολογισμός έναντι επικοινωνίας: Το πρόγραμμα στοχεύει στη μεγιστοποίηση του λόγου υπολογισμού προς επικοινωνία. Με την αποτελεσματική μετάδοση του μητρώου C και της διασκόρπισης του μητρώου Β, το πρόγραμμα μειώνει τη συχνότητα και τον όγκο της επικοινωνίας μεταξύ των διεργασιών, επιτρέποντας περισσότερο χρόνο για τον πραγματικό υπολογισμό.
Μοτίβα πρόσβασης στη μνήμη: Το πρόγραμμα έχει σχεδιαστεί έχοντας κατά νου αποτελεσματικά μοτίβα πρόσβασης στη μνήμη, ιδιαίτερα στον τρόπο αποθήκευσης και πρόσβασης των μητρώων κατά τη διάρκεια του πολλαπλασιασμού. Αυτό διασφαλίζει ότι η χρήση της προσωρινής μνήμης είναι βελτιστοποιημένη, βελτιώνοντας περαιτέρω την απόδοση.


