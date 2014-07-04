#!/opt/local/bin/octave -qf


arg_list = argv ();


arg_list{1}




allsamps=load(arg_list{1});

numsamples=str2num(arg_list{2})
numsteps=str2num(arg_list{3})


bestlabels=bestlabels;

%use 1 agent for interactive simulations
numagents=1
numexps=1  #usually 10

labelrate=5
j=0
labelno=1
for experno=0:(numexps-1)
 first=1+experno*numsteps*numsamples*numagents;
 last=first+numsteps*numsamples*numagents-1;

 first
 last
 size(allsamps)
 samps=allsamps(first:last,:);
 rsamps=reshape(samps',9,numsamples,numagents,numsteps);

 %lsamps(i,j,k) is ith dimension (of 9) the jth sample the kth step 
 % for the learner
 lsamps=squeeze(rsamps(:,:,1,:));
 if numagents > 1
   %lsamps(i,j,k) is ith dimension (of 9) the jth sample the kth step
   %for the simulator
   ssamps=squeeze(rsamps(:,:,2,:));
 else
     %just for consistency
     ssamps = lsamps;
 end


 maxx=max(max(max(lsamps([1,7],:))),max(max(ssamps([1,7],:))));
 maxy=max(max(max(lsamps([2,8],:))),max(max(ssamps([2,8],:))));
 minx=min(min(min(lsamps([1,7],:))),min(min(ssamps([1,7],:))));
 miny=min(min(min(lsamps([2,8],:))),min(min(ssamps([2,8],:))));



 for iter=1:numsteps

   lf=squeeze(lsamps(:,:,iter))';
   sf=squeeze(ssamps(:,:,iter))';
   mlf= mean(lf);
   msf=mean(sf);

   %plot the samples - client ids
   hold off
   plot(lf(:,7),lf(:,8),'bs',"markersize",6);
   hold on
   if numagents > 1
     plot(sf(:,7),sf(:,8),'rs',"markersize",6);
   end

   %the agent ids
   plot(lf(:,1),lf(:,2),'cs',"markersize",6);
   hold on
   if numagents > 1
     plot(sf(:,1),sf(:,2),'ms',"markersize",6);
   end


   %the means
   plot(mlf(:,1),mlf(:,2),'b^',"markersize",12);
   if numagents > 1
     plot(msf(:,1),msf(:,2),'r^',"markersize",12);
   end

   if rem(iter,labelrate)==0 
       labelagentl=bestlabels(labelno,:);
       labelclientl=bestlabels(labelno+1,:);
       labelagents=bestlabels(labelno+2,:);
       labelclients=bestlabels(labelno+3,:);
       labelno = labelno+4;
   end
   if iter>labelrate
       text(mlf(:,1),mlf(:,2)+1.5,labelagentl,'fontsize',16,'color','c')
       text(min(lf(:,7)),min(lf(:,8)),labelclientl,'fontsize',16,'color','b')
       if numagents > 1
	 text(msf(:,1),msf(:,2)+1.5,labelagents,'fontsize',16,'color','m')
	 text(min(sf(:,7)),min(sf(:,8)),labelclients,'fontsize',16,'color','r')
       end
   end
       
   %axis([minx maxx miny maxy]);
   axis([-4.3 4.3 -4.3 4.3]);
   
   xlabel('Evaluation','fontsize',16) 
   ylabel('Potency','fontsize',16) 
   %print(['samplesexp' int2str(experno) '-' int2str(iter) '.jpg'],'-djpg')
   print(['samplesexp' int2str(j) '.jpg'],'-djpg')
   %print(['samplesexp' int2str(j) '.fig'],'-dfig')
   j=j+1;
 end

end

