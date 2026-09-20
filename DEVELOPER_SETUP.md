# Developer setup (Guida rapida)

Questa guida copre i passi essenziali per iniziare a lavorare sul progetto.

1) Prerequisiti
- Unreal Engine 5.8 (installalo tramite Epic Games Launcher)
- Git (consigliata ultima versione) e Git LFS
- Visual Studio 2026 / VSCode / Rider (per C++ e editing)

2) Clonare il repo

```powershell
git clone https://github.com/<OWNER>/<REPOSITORY>.git
cd <REPOSITORY>
git lfs install
git lfs pull
```

3) Aprire il progetto
- Apri il file `.uproject` con Unreal Editor e segui le istruzioni per la prima generazione (se richiesta).

4) Locking LFS per asset binari
- Prima di modificare asset binari condivisi (mappe, materiali grandi), usa `git lfs lock <file>`.

ATTENZIONE (GitHub gratuito):
- Il piano gratuito dispone di quota limitata per Git LFS (storage/transfer). Evitare
  di caricare asset Unreal pesanti (`*.uasset`, `*.umap`, `*.uexp`, `*.ubulk`) sul
  repository principale a meno di avere quota LFS sufficiente.

Opzioni consigliate per gestire asset pesanti:
1. Usare uno storage esterno (S3, Google Drive) e versionare
	solo i riferimenti/metadata nel repo (ad esempio `.uasset` placeholders o
	file JSON che descrivono dove recuperare l'asset).
2. Creare un repository asset separato (privato) e sincronare solo asset piccoli
	nel repo principale.
3. Valutare sistemi specializzati per asset di gioco (Perforce, Plastic SCM)
	se il flusso di lavoro richiede frequenti grandi upload.


5) Commit e branch
- Crea branch con nome `feature/<id>-descrizione` o `hotfix/<descrizione>`.
- Apri PR e usa il template PR.

Nota specifica per questo repository:
- Il team ha deciso di consentire l'upload solo degli asset nella cartella
	`Content/Procedural_Labirint/`. Se modifichi o aggiungi file in quella
	cartella, questi possono essere tracciati nel repository principale. Tutti gli
	altri asset Unreal NON devono essere committati nel repo principale e vanno
	gestiti tramite storage esterno o repository asset dedicato.

Operazioni pratiche:
- Esegui `git lfs install` prima di lavorare su `Content/Procedural_Labirint`.
- Usa `git lfs lock <file>` quando inizi modifiche importanti su asset nella
	cartella Procedural_Labirint per evitare conflitti.

6) Self-hosted runner (per build UE)
- Per build/packaging pesanti, configure a Windows self-hosted runner (vedi GitHub Actions settings) e documenta la macchina.

7) Risorse utili
- Documentazione UE5.8
- Git LFS docs
