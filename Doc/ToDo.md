It's global issues ToDo - specific code issues are being marked 
'TODO: ...' withing code files, in VS one can view them in a 'Task List' (Ctrl+\,T) 

* **fixed:** (CImageCtrls::ConvertToDIB() creates temp for .bmp too) :
 loading .frn with ref to .bmp image causes the .bmp to be erazed on file close
* **fixed:** loading .zap/.frn using Recent Files List doesn't load fringes, image only
* design logic for placing the image relative to the sections in DOS .zap
* fix switching to '+' (add dot) on window's header click/focus restore
* file name must change after 'save as' otherwise it's 'save copy'
* show fringe number when hover over a dot or a fringe.
* undo for all the fringe manipulations
* allow moving dots in all the modes
* add line from the last dot to the cursor in '+' dot mode
* allow moving/transforming bounds while entering them or editing fringes
* allow moving image relative to the fringes/bound 
* make bounds' mode cross mark more contrast (red)
* 'apply' somtimes becomes inaccesible with a bound present
* allow few aperure masks, bound should be calculated as their outer limits
* scroll (grag) holding \<space\> key
* zoom with a mouse wheel and +/- keys
* break fringe (delete connection b/w dots)
* dots' vertical positions should not be restricted to the cut. 
  If some dot is moved outside its current cut new cut must be introduced.
  Maximum number of cuts corresponds to image's vertical resolution.
* review interferogramm loading logic (too vogue)
* ... should be added a lot here ...

### File loading logic (not clear enough)
```mermaid
sequenceDiagram
    participant User
    participant App as CDigitApp
    participant Doc as CImageDoc
    participant BaseDoc as CBaseImageDoc
    participant Utils as GetImageFileName
    participant ImgCtrl as CImageCtrls
    participant Digit as CDigitInfo

    User->>App: OnFileOpen() - Select file
    alt Image file (.bmp, .jpg, etc.)
        App->>Doc: OpenDocumentFile(image_file)
        Doc->>Doc: OnOpenDocument()
        Doc->>Doc: FileType() = T_PIC
        Doc->>BaseDoc: ReloadDocument(image_file)
        BaseDoc->>ImgCtrl: LoadImage(image_file)
        ImgCtrl->>ImgCtrl: ConvertToDIB() if needed
        ImgCtrl->>ImgCtrl: SECDib.LoadImage()
        BaseDoc->>BaseDoc: Create temp files
        BaseDoc-->>App: Return TRUE
        App->>Doc: TreatCmdLine() processing
        Doc-->>User: Display image document
        
    else ZAP/FRN file (.zap, .frn)
        App->>Doc: OpenDocumentFile(zap_file)
        Doc->>Doc: OnOpenDocument()
        Doc->>Doc: FileType() = T_ZAP/T_FRN
        Doc->>Utils: GetImageFileName(zap_file, cs)
        
        Note over Utils: FIXED PATH RESOLUTION
        Utils->>Utils: ReadZAPData/ReadFRNData
        Utils->>Utils: Extract directory from zap_file
        Utils->>Utils: Build full image path
        Utils->>Utils: Check file existence
        
        alt Image file exists
            Utils-->>Doc: Return valid image_path
            Doc->>BaseDoc: ReloadDocument(image_path)
            BaseDoc->>ImgCtrl: LoadImage(image_path)
            BaseDoc->>BaseDoc: Create temp files
            BaseDoc-->>Doc: Return TRUE
        else Image file missing
            Utils-->>Doc: Return empty image_path
            Doc->>Doc: ReadZAPData/ReadFRNData
            Note over Doc: Load interferogram data only
            Doc-->>App: Return TRUE (doc without image)
        end
        
        App->>Doc: TreatCmdLine() processing
        Doc->>Digit: Load(zap_file)
        Note over Digit: Load interferogram data
        Doc-->>User: Display document (with or without image)
    end
```

```mermaid

sequenceDiagram
    participant App as CDigitApp
    participant Doc as CImageDoc
    participant Utils as GetImageFileName (FIXED)

    App->>Doc: OpenDocumentFile("C:\Data\exp.zap")
    Doc->>Utils: GetImageFileName("C:\Data\exp.zap", cs)
    
    Utils->>Utils: ReadZAPData("C:\Data\exp.zap", IntInfo)
    Note over Utils: Finds ImageFileName = "image.bmp"
    
    Utils->>Utils: Extract directory = "C:\Data\"
    Utils->>Utils: Build full path = "C:\Data\image.bmp"
    Utils->>Utils: Check file existence → SUCCESS
    
    Utils-->>Doc: PathName = "C:\Data\image.bmp", cs = (width, height)
    
    Doc->>BaseDoc: ReloadDocument("C:\Data\image.bmp")
    Note over Doc: Image loads successfully!
```
