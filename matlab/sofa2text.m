%% Script to convert BRIRs in sofa file format to txt files readable by vas_Library.
%
% Dependencies: SOFA API https://sourceforge.net/projects/sofacoustics/
%
% Place this file in your sofa directory and run it. All sofas will be
% converted to txt.
%
% 30.10.2019, Christoph Boehm, c.boehm@tu-berlin.de
% Audio Communication Group, Technical University Berlin

close all; clear all; clc;

%% Read / Write

% Get all sofa files
directory = dir('*.sofa');

% Truncate to multiple of buffer size
buffer = 1024;
truncToBufferSize = 1;
    

% Read all files and save to txt
for m = 1:length(directory)
    
    % Disp status
    fprintf('[sofa2txt] Converting file %d of %d\n', m, length(directory));
     
    % Metadata
    elevationstride = 1;
    azimuthstride = 3;
    directionformat = 'azimuth'; % 'azimuth' or 'single' or 'multi'
    audioformat = 'stereo';
    lineformat = 'ir';
   
    % Read Sofa
    [filepath,currentFile,ext] = fileparts(directory(m,1).name);
    H1 = SOFAload([currentFile ext]);
    
    % If file is tail select meta
    if ~isempty(strfind(currentFile,'tail')); 
        tail = 1; 
        elevationstride = 1;
        azimuthstride = 1;
        directionformat = 'single';
    end
    
    % Truncate to buffer size
    if truncToBufferSize == 1
        IRLength = length(H1.Data.IR(1,1,:))-mod(length(H1.Data.IR(1,1,:)),buffer);
    else
        IRLength = length(H1.Data.IR(1,1,:));
    end
    
    % Header for text file
    header = sprintf('metadata {\nlength %d\ndirectionformat %s\naudioformat %s\nlineformat %s\nelevationstride %d\nazimuthstride %d\n}', IRLength, directionformat, audioformat, lineformat, elevationstride, azimuthstride);
 
    % Open txt file
    fileID = fopen([currentFile '.txt'],'wt');
    fprintf(fileID, header);       

    % Write IRs and meta
    for channel = 1:2
  
        if channel == 1; kanal = 'left'; else kanal = 'right'; end

        fprintf(fileID, '\n%s {\n', kanal);
        for i = 1:length(H1.ListenerView(:,1))
            
            fprintf(fileID,'%d, ',H1.ListenerView(i,1));
            for n = 1:length(H1.Data.IR(i,1,:))
                fprintf(fileID,'%f, ', H1.Data.IR(i,channel,n));
            end
            if i ~= length(H1.ListenerView(:,1)); fprintf(fileID,'\n'); end
        end
        fprintf(fileID, '}');

    end
    
fclose(fileID);

end

disp('[sofa2txt] Done!');

