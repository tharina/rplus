# Algorithm Engineering Programmmieraufgaben — Range Search

Implementieren und evaluieren Sie effiziente Datenstrukturen für 2D orthogonal range search in C++11/14.
Nutzen Sie dafür das vorgegebene Framework.
Das Framework gibt ein Interface vor, das Sie einhalten müssen ([`range_search.h`](range_search.h)).
Eine Beispiel-Implementierung einer naiven Suche finden Sie in [`naive.h`](naive.h).
Da es eine Vielzahl verschiedener Möglichkeiten gibt, kann (und soll) die Aufgabe von mehreren Personen bearbeitet werden.

Mögliche Varianten beinhalten:
- Quad trees
- k-d trees
- Range trees
- Range trees w/ Fractional Cascading
- R trees
- R\* trees
- Hilbert R trees
- Wavelet trees

## Implementierung

Ihre Implementierung sollte generisch sein und den Punkt-Typ als Template-Parameter entgegennehmen.
Es ist allerdings ausreichend, wenn Sie nur 2D-Koordinaten unterstützen (nicht alle Datenstrukturen funktionieren für beliebig viele Dimensionen).
Sie können auf die Koordinaten mittels `p[0]` und `p[1]` zugreifen.

Sie müssen das vorgegebene Interface ([`range_search.h`](range_search.h)) implementieren.
Darin vorgegeben sind folgende Methoden:

- [`void assign(const std::vector<Point>&)`](range_search.h#L21)

     Setzt die zugrundeliegende Punktmenge.
     Hier sollten Sie alle Vorberechnungsschritte ausführen und Ihre Datenstruktur erstellen.

- [`void reportRange(const Point&, const Point&, std::vector<Point>&)`](range_search.h#L24)

     Führt eine orthogonal range reporting query aus.
     Die Punkte spannen ein Rechteck auf, wobei der erste die untere linke Ecke darstellt und der zweite die rechte obere.
     Alle Punkte der Grundmenge, die sich innerhalb dieses Rechtecks befinden, werden dem vector hinzugefügt.

- [`size_t countRange(const Point&, const Point&)`](range_search.h#L27)

     Führt eine orthogonal range counting query aus.
     Wie oben, allerdings werden die Punkte nur gezählt anstatt ausgegeben, was oft effizienter implementiert werden kann.

Darüber hinaus müssen Sie Ihre Implementierung registrieren.
Tragen Sie dazu in der Datei [`contenders.h`](contenders.h) sowohl die entsprechende `#include`-Anweisung als auch die zu testenden Datenstrukturen ein.
Dazu geben Sie einen Namen und eine Factory-Funktion an, analog zum enthaltenen Beispiel.
Sollte Ihre Datenstruktur verschiedene Konfigurationen unterstützen (wie zum Beispiel Splitting-Strategien), können und sollten Sie mehrere Einträge erstellen.

Hinweis: Das Interface ist bewusst einfach gehalten, um die Komplexität zu verringern und Ihnen möglichst wenig Steine in den Weg zu legen.
Das geht zu Kosten der Flexibilität; es steht Ihnen aber frei, weitere Methoden zu definieren, um z.B. mit beliebigen Input- und Output-Iteratoren zu arbeiten.
Darüber hinaus sind virtuelle Funktionen natürlich mit einem gewissen Performance-Overhead behaftet.
Dieser betrifft aber alle Implementierungen im gleichen Maße und hat daher keine nennenswerten Auswirkungen auf die Vergleichsmessungen.

## Voraussetzungen

Da das Framework Funktionalitäten aus dem C++11-Standard verwendet muss ein entsprechender Compiler zum Einsatz kommen.
Empfohlen werden der GNU Compiler `g++` ab Version 4.8 oder das C++-Frontend `clang++` des LLVM Compilers ab Version 3.4.
Bei diesen älteren Versionen ist es unter Umständen nötig, den Standard in der [`Makefile`](Makefile#L4) auf `-std=c++11` zu ändern.
Da das Framework einige POSIX-Funktionen verwendet kann es aktuell nicht unter Windows kompiliert werden.

Folgende Abhängigkeiten werden benötigt:
- [libPAPI](http://icl.cs.utk.edu/papi/) zur Messung von Cache- und Branch-Misses
- [malloc\_count](https://github.com/bingmann/malloc_count) zur Messung des Speicherverbrauchs (im Repository bereits enthalten)
- Optional [sqlplot-tools](https://github.com/bingmann/sqlplot-tools) zur Erzeugung von Plots der Messergebnisse

Als Buildsystem kommt GNU make zum Einsatz.
Dieses erzeugt standardmäßig die Binaries `bench` und `bench_malloc`, die Zeit-, Performance-Counter-, sowie Speichermessungen durchführen.
Darüber hinaus können Sie mit Hilfe von `sanitize` den [AddressSanitizer](http://clang.llvm.org/docs/AddressSanitizer.html) von Clang verwenden, um mögliche Speicherlecks oder Zugriffsverletzungen zu finden.

## Verwendung

Bei Ausführung ohne Parameter führt `bench` Laufzeit-, Performance-Counter-, und Speichermessungen für eine Reihe von Benchmarks verschiedener Größen aus.
Mit Hilfe von Kommandozeilen-Parametern können Sie dies Einschränken, was besonders bei der Entwicklung hilfreich ist.
Mögliche Parameter können Sie mittels `bench -h` ausgeben lassen.

Benchmark-Ergebnisse werden auf der Konsole ausgegeben und in maschinenlesbarer Form im Verzeichnis `results/` abgespeichert.
Sie können daraus mit Hilfe von [sqlplot-tools](https://github.com/bingmann/sqlplot-tools) einfach Plots erstellen.
Führen Sie dazu `make` im Unterverzeichnis `plots/` aus.
Ein vorkompiliertes Binary ist dort bereits abgelegt; sollte dieses auf Ihrem System nicht funktionieren, müssen Sie sqlplot-tools manuell installieren.

Standardmäßig werden nur eine Auswahl an möglichen Plots erstellt, die entweder Benchmarks oder Datenstrukturen vergleichen lassen.
Weitere Plots sind möglich, indem Sie den entsprechenden Dateinamen angeben; alternativ können Sie mittels `make everything` alle Messungen visualisieren.
In der Ausarbeitung sollten Sie sich natürlich auf einige wenige, interessante Plots konzentrieren.

